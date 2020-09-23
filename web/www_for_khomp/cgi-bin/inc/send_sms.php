<?php
include_once("/www/cgi-bin/inc/function.inc");
/********************************************************************************************
 *
 *	class: SendSMS
 *
 *	USING: SendSMS->send($MSG, $SPANS, $NUMBERS, $ATTEMPT, $VERBOSE, $REPEAT)
 *	
 *	The process flow:
 *		1.init(set cluster if necessary; get valid spans, valid number, split sms);
 *		2.fork child process for every valid spans;
 *		3.child process
 *			3.1.get number using shm;
 *			3.2.send the split sms(a_msg);
 *			3.3.repeat send if necessary(repeat);
 *			3.4.resend if failed if necessary(attempt);
 *
 *	shared memory var:
 *		1.phone number id, point to the current phone number being send;
 *		2.the total number of child process;
 *		3.the number of child process who has send over;
 *
 *	get phone number process:
 *		1.get file lock;
 *		2.read shm var phone number id;
 *		3.phone number id add 1;
 *		4.write shm var phone number id;
 *		5.release file lock;
 *
 *******************************************************************************************/



class SendSMS {
	//The original SMS information, set by send()
	private $_verbose = 0;
	private	$_repeat = 0;
	private	$_attempt = 0;
	private $_msg = "";
	public $_spans = "";
	private $_numbers = "";
	private $_flash_sms = "";

	//The dealt SMS information, set by init()
	const SIZE7BIT = 160;
	const SIZE8BIT = 70;
	private $a_msg = array();	//msg split by 160/70
	private $a_numbers = array();	//valid phonenumber array
	public $a_spans = array();	//spans: $a_spans[1~5][0~4];
	private $number_id = 0;

	/*cluster information : 
	 *	$a_spansinfo['boards_ip'][1~5];
	 *	$a_spansinfo['valid'][1~5][0~4];
	 */
	private $a_spansinfo = array();	
	public $timeout = 20;	

	//Share memory var
	private $shm_id = null;
	private $shm_number_id_key = 1;
	private $shm_pid_sum_key = 2;
	private $shm_send_over_key = 3;
	private $shm_send_total_key = 4;
	private $shm_send_success_key = 5;
	private $shm_send_fail_key = 6;
	private $lock_file_path = __FILE__;

	//private $debug = 3;
	private $debug = false;

	public function __construct() {
		//ignore_user_abort(false);
		set_time_limit(0);
		mb_internal_encoding('UTF-8');
	}

	public function trace($context) {
		echo "$context";
		echo "<br>";
		ob_flush();
		flush();
	}

	public function trace_sms($msg, $to, $span, $repeat, $attempt, $result, $flag = "tr") {
		if($flag == "tr"){		
			echo "<tr align='center' style='background-color: rgb(232, 239, 247);'>";
			echo "<td align='left' style='width:45%;word-break:break-all;'>$msg</td>";
			echo "<td style='width:15%'>$to</td>";
			echo "<td style='width:12%'>$span</td>";
			echo "<td style='width:10%'>$repeat</td>";
			echo "<td style='width:10%'>$attempt</td>";
			echo "<td style='width:8%'>$result</td>";
			echo "</tr>";
		}else if($flag == "table_start"){
			echo "<table style='width:100%;font-size:12px;border:1px solid rgb(59,112,162);'>";
			echo "<tr style='background-color:#D0E0EE;height:26px;'>";
			echo "<th style='width:45%;word-break:break-all;'>$msg</th>";
			echo "<th style='width:15%'>$to</th>";
			echo "<th style='width:12%'>$span</th>";
			echo "<th style='width:10%'>$repeat</th>";
			echo "<th style='width:10%'>$attempt</th>";
			echo "<th style='width:8%'>$result</th>";
			echo "</tr>";
		}else if($flag == "table_end"){
			echo "</table>";
		}
		
		ob_flush();
		flush();
	}

	private function trace_sms_statistics($total, $success, $fail){
		$total   = $success + $fail;		
		$table   = "<b>Statistics Report</b>";
		$table  .= "<table style='width:100%;font-size:12px;border:1px solid rgb(59,112,162);'>";
		$table 	.=	"<tr style='background-color:#D0E0EE;height:26px;'>";
		$table 	.=		"<th style='width:30%'>Total</th>";
		$table 	.=		"<th style='width:30%'>Success</th>";
		$table 	.=		"<th style='width:30%'>Fail</th>";
		$table 	.=	"</tr>";
		$table 	.=	"<tr align='center' style='background-color: rgb(232, 239, 247);'>";
		$table 	.=		"<td style='width:30%'>$total</td>";
		$table 	.=		"<td style='width:30%'>$success</td>";
		$table 	.=		"<td style='width:30%'>$fail</td>";
		$table 	.=	"</tr>";
		$table 	.= "</table>";

		$script  = "<script type='text/javascript'>";
		$script .= 	"$(function(){\$('#report').after(\"$table\");});";
		$script .= "</script>";
		
		echo $script;
		

		ob_flush();
		flush();
	}

	public function is_brower_colose() {

		if(connection_status() != 0)
			return true;

		echo ' ';
		flush();
		ob_flush();
		
		return false; 
	}

	public function isAscii($MSG) {
		return (strlen($MSG) == mb_strlen($MSG));
	}


	public function dealMsg($MSG) {
		if($this->isAscii($MSG)){
			$size = self::SIZE7BIT;
			$csms_size = self::SIZE7BIT - 8;
		} else {
			$size = self::SIZE8BIT;
			$csms_size = self::SIZE8BIT - 3;
		}

		$len = mb_strlen($MSG);

		if($len <= $size) {
			$this->a_msg[] = $MSG;
		} else {
			//Divide string
			$tmp = (int)($len/$csms_size);
			for($i=0; $i<$tmp; $i++) {
				$msg = mb_substr($MSG, $i*$csms_size, $csms_size);
				$this->a_msg[] = $msg;
			}

			//Remain string
			$tmp = (int)($len%$csms_size);
			if($tmp !=0 ) {
				$msg = mb_substr($MSG, $i*$csms_size, $tmp);
				$this->a_msg[] = $msg;
			}
		}

		if(empty($this->a_msg)) {
			$this->trace("No has SMS to send!"); 
			return false;
		}

		return true;
	}
	

	public function dealSpans($SPANS) {
		if($SPANS == "auto") {
			$this->a_spans=$this->a_spansinfo['valid'];
			return true;
		}

		$span_flag = 0;
		foreach($SPANS as $key1=>$board) {
			foreach($board as $key2=>$span){
				if(!isset($this->a_spansinfo['valid'][$key1][$key2]))
					unset($SPANS[$key1][$key2]);
			}
			if(!empty($SPANS[$key1]))$span_flag++;
		}
	
		$this->a_spans = $SPANS;
		if($span_flag==0) {
			$this->trace("No valid spans to send!");
			return false;
		}
		return true;
	}

	public function dealNumbers($NUMBERS) {
		$len = strlen($NUMBERS);
		$start = 0;

		for($i=0; $i<$len; $i++) {
			if($NUMBERS[$i] == ' ' || $NUMBERS[$i] == ',' || $NUMBERS[$i] == ';'|| $NUMBERS[$i] == '.' || $NUMBERS[$i] == "\r" || $NUMBERS[$i] == "\n") {
				$number = trim(substr($NUMBERS,$start,$i-$start));
				if($number != '') {
					//if ($this->isValidNumber($number) && !$this->hasSame($this->a_numbers, $number)) {
					if ($this->isValidNumber($number)){
						$this->a_numbers[] = $number;
					}
				}
				$start = $i+1;
			}
		}
		$number = trim(substr($NUMBERS,$start,$len-$start));
		if($number != '') {
			//if ($this->isValidNumber($number) && !$this->hasSame($this->a_numbers, $number)) {
			if ($this->isValidNumber($number)) {
				$this->a_numbers[] = $number;
			}		
		}

		if(empty($this->a_numbers)){
			return false;
		}

		return true;
	}


	public function isValidSpan($span) {
		foreach($this->a_vspans as $each) {
			if($each === $span)
				return true;
		}

		return false;
	}


	public function isValidNumber($number) {
		return preg_match('/^[+0-9][0-9]+$/',$number);
	}


	public function hasSame($array, $member) {
		foreach($array as $each) {
			if($each === $member) {
				return true;
			}
		}

		return false;
	}

	public function spans_init() {
		global $__GSM_SUM__;
		global $__GSM_HEAD__;			//gsm head 
		global $__BRD_HEAD__;			//Board head
		global $__BRD_SUM__;

		$valid_flag = 0;

		exec("asterisk -rx \"gsm show spans\" || echo 'error'",$output);
		if($this->debug & 2){
			if(is_string($output))
				$this->trace("exec output:$output");
			if(is_array($output))
				print_r($output);
			$this->trace("");
		}
		if(end($output) == 'error') {
			$this->trace( "Call board-1 asterisk failed");
		}else{
			foreach($output as $each) {
				if(strstr($each, 'Up, Active')) {
					sscanf($each, "GSM span %d: ",$span);
					if(is_integer($span))
						$this->a_spansinfo['valid'][1][$span] = $span;
				}
			}
			if(!empty($this->a_spansinfo['valid'][1]))
				$valid_flag++;
		}

		//init cluster slave spans if it has been set.
		$cluster_info = get_cluster_info();                                                                                                    
		if($cluster_info['mode'] == 'master') {
			for($b=2; $b<=$__BRD_SUM__; $b++) {
				if($cluster_info[$__BRD_HEAD__.$b.'_ip'] != '') {
					$this->a_spansinfo['boards_ip'][$b] = $cluster_info[$__BRD_HEAD__.$b.'_ip'];

					$output = request_slave($this->a_spansinfo['boards_ip'][$b], "astcmd:gsm show spans", 1);
					$output = explode("\n", $output);
					if($this->debug & 2){
						if(is_string($output))
							$this->trace("request_slave output:$output");
						if(is_array($output))
							print_r($output);
						$this->trace("");
					}
					if(end($output) == 'error') {
						$this->trace( "Call ".$__BRD_HEAD__.$b." asterisk failed");
					}else{
						foreach($output as $each) {
							if(strstr($each, 'Up, Active')) {
								sscanf($each, "GSM span %d: ",$span);
								if(is_integer($span))
									$this->a_spansinfo['valid'][$b][$span] = $span;
							}
						}
						if(!empty($this->a_spansinfo['valid'][$b]))
							$valid_flag++;
					}
				}   
			}   
		}
		if($this->debug & 2){
			if(is_string($this->a_spansinfo))
				$this->trace("a_spansinfo:".$this->a_spansinfo." valid flag=$valid_flag");
			if(is_array($this->a_spansinfo['valid']))
				print_r($this->a_spansinfo['valid']);
			$this->trace("");
		} 
		if($valid_flag == 0) {
			$this->trace("No has valid spans");
			return false;
		}else{
			return true;
		}
	}

	private function shm_init($key='', $memsize=2048) {
		if($key == '')
			$key = ftok(__FILE__, 'x');
		$shm_id = shm_attach($key);
		shm_remove($shm_id);
		$shm_id = shm_attach($key, $memsize);
		if(!$shm_id){
			if($this->debug & 2){
				echo "create shm error! shm_id=".$shm_id;
			}
			return false;
		}else{
			if($this->debug & 2){
				echo "shm_id=".$shm_id;
				$this->trace("");
			}
			return $shm_id;
		}
	}

	private function shm_delete($shm_id) {
		shm_remove($shm_id);
	}

	private function lock_file($file_path){
		$new_name = str_replace("/","_",$file_path);
		$lock_path = "/tmp/lock/" . $new_name . ".lock";
		$fh = fopen($lock_path, "w+");
		if($fh <= 0) {
			return -1; 
		}   
		flock($fh,LOCK_EX);

		return $fh;
	}

	private function unlock_file($fh){
		if($fh > 0) {
			flock($fh,LOCK_UN);
			fclose($fh);
		}   
	}

	public function init() {
		if(!$this->spans_init())
			return false;

		if(!$this->dealMsg($this->_msg))
			return false;

		if(!$this->dealSpans($this->_spans))
			return false;

		if(!$this->dealNumbers($this->_numbers))
			return false;
		
		if(!$this->shm_id = $this->shm_init())
			return false;	

		$ret1 = shm_put_var($this->shm_id, $this->shm_number_id_key, 0);
		$ret2 = shm_put_var($this->shm_id, $this->shm_pid_sum_key, 0);
		$ret3 = shm_put_var($this->shm_id, $this->shm_send_over_key, 0);
		$ret4 = shm_put_var($this->shm_id, $this->shm_send_total_key, 0);
		$ret5 = shm_put_var($this->shm_id, $this->shm_send_success_key, 0);
		$ret6 = shm_put_var($this->shm_id, $this->shm_send_fail_key, 0);
		if(!$ret1 || !$ret2 || !$ret3 || !$ret4 || !$ret5 || !$ret6){
			if($this->debug >= 2){
				echo "shm put var error! shm_id=".$this->shm_id;
			}
			return false;
		}

		return true;
	}

	private function release(){
		$this->shm_delete($this->shm_id);
	}

	private function get_lock_number(){
		if($this->debug & 1){
			echo getmypid()."in get_lock_number()";
			$this->trace("");
		}

		$fh = $this->lock_file($this->lock_file_path);
		$number_id = shm_get_var($this->shm_id, $this->shm_number_id_key);
		$this->number_id = $number_id;
		$number_id++;
		$ret = shm_put_var($this->shm_id, $this->shm_number_id_key, $number_id);
		$this->unlock_file($fh);

		if(!$ret)
			exit("shm put var error");
		if($this->debug & 1){
			echo getmypid()."in get_lock_number() number_id=".$this->number_id;
			$this->trace("");
		}
		
		if(isset($this->a_numbers[$this->number_id])){
			if($this->debug & 1){
				echo getmypid()."in get_lock_number() number=".$this->a_numbers[$this->number_id];
				$this->trace("");
			}
			return $this->a_numbers[$this->number_id];
		}else{
			$fh = $this->lock_file($this->lock_file_path);
			$send_over = shm_get_var($this->shm_id, $this->shm_send_over_key);
			if($this->debug & 1){
				echo getmypid()."in get_lock_numer() send_over=".$send_over;
				$this->trace("");
			}
			$send_over++;
			$ret = shm_put_var($this->shm_id, $this->shm_send_over_key, $send_over);
			$this->unlock_file($fh);

			if(!$ret)
				exit("shm put var error");
			return false;
		}
	}

	private function is_send_over(){
		$fh = $this->lock_file($this->lock_file_path);
		$send_over = shm_get_var($this->shm_id, $this->shm_send_over_key);
		$pid_sum = shm_get_var($this->shm_id, $this->shm_pid_sum_key);
		$this->unlock_file($fh);

		if($this->debug & 1){
			$this->trace(getmypid()."in is_send_over() send_over=$send_over pid_sum=$pid_sum");
			$this->trace("");
		}
		if($send_over >= $pid_sum)
			return true;
		else
			return false;
	}

	private function shm_var_increase($key){

		$fh = $this->lock_file($this->lock_file_path);
		$value = shm_get_var($this->shm_id, $key);
		if($this->debug & 1){
			echo getmypid()." in shm_var_increase() key = $key value = ".$value;
			$this->trace("");
		}
		$value ++;
		$ret = shm_put_var($this->shm_id, $key, $value);
		$this->unlock_file($fh);
	}

	private function send_sms($board, $span, $number, $msg, $repeat, $attempt, $csms=0, $flag=0, $smscount=0, $smssequence=0) {
		global $__GSM_SUM__;
		global $__GSM_HEAD__;			//gsm head 
		global $__BRD_HEAD__;			//Board head
		global $__BRD_SUM__;

		if($this->is_brower_colose()) {
			exit(0);
		}

		if($this->_verbose > 2)
			$this->trace("Start sending \"$msg\" to \"$number\" from span ".$__BRD_HEAD__.$board."-".$__GSM_HEAD__.$span);
		$save_msg = $msg;
		
		if (isset($msg[0])) {
			if ($msg[0] == '$') {  // for Escape character $
				$msg = "\\" . $msg;
			}
		}
		
		
		if($board == 1) {
			if(!$csms) {
				exec("asterisk -rx \"gsm send sync sms \\\"$span\\\" \\\"$number\\\" \\\"$msg\\\" ".$this->timeout." ".$this->_flash_sms." \" || echo $?",$output);
			} else {
				exec("asterisk -rx \"gsm send sync csms \\\"$span\\\" \\\"$number\\\" \\\"$msg\\\" $flag $smscount $smssequence ".$this->timeout." ".$this->_flash_sms."\" || echo $?",$output);
			}
		} elseif($board > 1) {
			if(!$csms) {
				$output = request_slave($this->a_spansinfo['boards_ip'][$board], "astcmd:gsm send sync sms \"$span\" \"$number\" \\\"$msg\\\" ".$this->timeout." ".$this->_flash_sms, $this->timeout+2);
			} else {
				$output = request_slave($this->a_spansinfo['boards_ip'][$board], "astcmd:gsm send sync csms \"$span\" \"$number\" \\\"$msg\\\" $flag $smscount $smssequence ".$this->timeout." ".$this->_flash_sms, $this->timeout+2);
			}
			$output = explode("\n", $output);
		}
		$msg = $save_msg; 
		if($this->debug & 2){
			$this->trace("");
			$this->trace(getmypid()."in send_sms: $board, $span, $number, $msg, $repeat, $attempt "." timeout=".$this->timeout.", output: ");
			print_r($output);
			$this->trace("");
		}
		if(is_array($output) && isset($output[0])){
			$a_output = explode(" ", trim($output[0]));
			$output = end($a_output);
			if($output == "SUCCESSFULLY"){
				//web log
				$this->trace_sms($msg, $number, get_gsm_name_by_channel($span,$board,false), $repeat, $attempt, "SUCCESS");
				$this->shm_var_increase($this->shm_send_success_key);
				return 0;
			}else if($output == "FAILED"){
				//web log
				$this->trace_sms($msg, $number, get_gsm_name_by_channel($span,$board,false), $repeat, $attempt, "FAILED");
				$this->shm_var_increase($this->shm_send_fail_key);
				$attempt++;
				if($attempt > $this->_attempt){
					return -1;
				}else{
					return $this->send_sms($board, $span, $number, $msg, $repeat, $attempt, $csms, $flag, $smscount, $smssequence);	//fail and resend
				}
			}else{
				//web log
				$this->trace_sms($msg, $number, get_gsm_name_by_channel($span,$board,false), $repeat, $attempt, $output);
				$this->shm_var_increase($this->shm_send_fail_key);
				$attempt++;
				if($attempt > $this->_attempt){
					return -1;
				}else{
					return $this->send_sms($board, $span, $number, $msg, $repeat, $attempt, $csms, $flag, $smscount, $smssequence);	//fail and resend
				}
			}
		}else{
			//web log
			$this->trace_sms($msg, $number, get_gsm_name_by_channel($span,$board,false), $repeat, $attempt, "UNDEFINED");
			$this->shm_var_increase($this->shm_send_fail_key);
			$attempt++;
			if($attempt > $this->_attempt){
				return -1;
			}else{
				return $this->send_sms($board, $span, $number, $msg, $repeat, $attempt, $csms, $flag, $smscount, $smssequence);	//fail and resend
			}
		}
	}

	private function child_process_send(){
		global $__GSM_SUM__;
		global $__GSM_HEAD__;			//gsm head 
		global $__BRD_HEAD__;			//Board head
		global $__BRD_SUM__;
		$pid_sum = 0;

		foreach($this->a_spans as $key_board=>$board){
			foreach($board as $key_span=>$span){
				$pid = pcntl_fork();
				if($pid < 0){
					exit("fork error");
				}else if($pid > 0){
					$pids[]=$pid;
					$this->shm_var_increase($this->shm_pid_sum_key);
					if($this->debug & 2){
						$pid_sum++;
						$this->trace(getmypid()." pid sum ".$pid_sum);
					}
				}else if($pid == 0){
					if($this->debug & 1){
						$this->trace("");
						echo "in child process span=$__BRD_HEAD__$key_board-$__GSM_HEAD__$key_span ".getmypid();
						$this->trace("");
						$this->trace("");
					}
					while($number = $this->get_lock_number()){
						if($this->debug & 1){
							$this->trace(getmypid()."get number : $number\n");
							$this->trace("");
						}
						for($i=1; $i<=$this->_repeat; $i++) {

							$smscount = count($this->a_msg);
							if($smscount >= 2) {
								$flag = rand(0,255);
								$smssequence = 1;
							}

							foreach($this->a_msg as $msg) {
								if($this->debug & 1){
									$this->trace(getmypid()." send_sms($key_board, $key_span,$number, $msg, $i, 0)");
									$this->trace("");
								}
								$this->shm_var_increase($this->shm_send_total_key);
								if($smscount >= 2) {
									$ret = $this->send_sms($key_board, $key_span, $number, $msg, $i, 0, 1, $flag, $smscount, $smssequence);
									$smssequence++;
								} else {
									$ret = $this->send_sms($key_board, $key_span, $number, $msg, $i, 0);
								}
								if($this->debug & 1){
									$this->trace(getmypid()." send_sms($key_board, $key_span,$number, $msg, $i, 0) ok");
									$this->trace("");
								}
								if($ret<0)
									break;
							}
						}
						if($this->debug & 1){
							$this->trace("");
							$this->trace("");
							echo "----span=$__BRD_HEAD__$key_board-$__GSM_HEAD__$key_span get new number----";
							$this->trace("");
						}
					}
					if($this->debug & 1){
						$this->trace("");
						$this->trace("###span=$__BRD_HEAD__$key_board-$__GSM_HEAD__$key_span wait exit###".getmypid());
						$this->trace("");
					}
					while(!$this->is_send_over()){
						sleep(1);
						if($this->is_brower_colose()) {
							exit(0);
						}
					}
					if($this->debug & 1){
						$this->trace("");
						$this->trace("###span=$__BRD_HEAD__$key_board-$__GSM_HEAD__$key_span exit###".getmypid());
						$this->trace("");
					}
					//return;
					exit(0);
					if($this->debug & 1){echo "child process ".$getmypid()." exit\n";}
				}
			}
		}
		if(isset($pids) && is_array($pids)) {		
			foreach($pids as $pid){
				pcntl_waitpid($pid, $status);
			}
		}
		if($this->debug & 1){echo "parent process exit\n";}
		return;
	}

	public function send($MSG, $SPANS, $NUMBERS, $ATTEMPT, $VERBOSE, $REPEAT,$FLASH_SMS) {
		$this->_repeat = $REPEAT;
		$this->_verbose = $VERBOSE;
		$this->_attempt = $ATTEMPT;
		$this->_spans = $SPANS;
		$this->_numbers = $NUMBERS;
		$this->_flash_sms = $FLASH_SMS;
		$msg = $MSG;
		//replace '\' (must first then replace '"')
		$msg = str_replace("\\","\\\\\\\\",$msg);
		//replace '"'
		$msg = str_replace("\"","\\\\\\\"",$msg);
		//replace '`'
		$msg = str_replace("`","'",$msg);
		$this->_msg = $msg;

		if($this->debug & 1){
			$this->trace("");
			$this->trace("_repeat");print_r($this->_repeat);$this->trace("");
			$this->trace("_verbose");print_r($this->_verbose);$this->trace("");
			$this->trace("_attempt");print_r($this->_attempt);$this->trace("");
			$this->trace("_spans");print_r($this->_spans);$this->trace("");
			$this->trace("_numbers");print_r($this->_numbers);$this->trace("");
			$this->trace("_msg");print_r($this->_msg);$this->trace("");
			$this->trace("");
		}

		if(!$this->init())
			return false;

		if($this->debug & 1){
			$this->trace("");
			$this->trace("a_msg");print_r($this->a_msg);$this->trace("");
			$this->trace("a_spans");print_r($this->a_spans);$this->trace("");
			$this->trace("a_numbers");print_r($this->a_numbers);$this->trace("");
			$this->trace("a_spansinfo");print_r($this->a_spansinfo);$this->trace("");
			$this->trace("");
		}

		//$this->trace("SMS Sending ... ");
		echo "<br/>";
		$this->trace("<div id=\"report\"></div>");
		$this->trace("<b>Detail Report</b>");
		$this->trace_sms("Message", "Destination Number","Port", "Repeat times", "Attempt times", "Result", "table_start");
		$this->child_process_send();
		$this->trace_sms("Message", "Destination Number","Port", "Repeat times", "Attempt times", "Result", "table_end");

		$fh = $this->lock_file($this->lock_file_path);
		$send_total = shm_get_var($this->shm_id, $this->shm_send_total_key);
		$send_success = shm_get_var($this->shm_id, $this->shm_send_success_key);
		$send_fail = shm_get_var($this->shm_id, $this->shm_send_fail_key);
		$this->unlock_file($fh);
		$this->trace("");
		$this->trace_sms_statistics($send_total, $send_success, $send_fail);

		$this->release();
	}
}

?>
