<?php
require_once("/www/cgi-bin/inc/language.inc");
include_once("/www/cgi-bin/inc/userdb.php");
include_once("/www/cgi-bin/inc/redis.inc");

$language = get_web_language_cache('/tmp/web/language.cache');

session_start();

$login = new Login();
$login->user_already_login();//User already Login
$dis_time = $login->get_dis_time();

if($_POST){
	$login->login();
}

?>

<html>
	<head>
		<meta charset="utf-8">
		<title>login</title>
		<link rel="stylesheet" type="text/css" href="/css/style-new.css">
		<link rel="stylesheet" type="text/css" href="/css/index.css?v=1.0">
		<script src="/js/jquery.js"></script>
	</head>

	<div id="background">
		<div id="background_top"></div>
	</div>
	
	<div id="login_content">
		<div class="wel">
			<img src="/images/logo.jpeg" />
		</div>
		<form action="<?php echo get_self();?>" method="post" class="table">
			<div>
				<div class="user">
					<span id="login_help" style="font-size:14px;color:red;display:none;">
					<?php if(isset($_SESSION["error_report"])) echo $_SESSION["error_report"];?>
					</span>
					<div id="yonghu">
						<img src="/images/yhm.png">
						<input type="text" name="username" placeholder="<?php echo language("Username");?>">
					</div>
				</div>			
				<div class="user">
					<div id="yonghu">
						<img src="/images/mm.png">
						<input type="password" name="password" placeholder="●●●●●●">
					</div>
				</div>
				
				<div class="user">
					<button class="btn" type="submit" name="login" ><?php echo language('Login');?></button>
				</div>
			</div>
			<input type="hidden" name="error_report" value="" />
		</form>
	</div>
</html>

<script>
<?php
if(isset($_POST['username'])){
?>
$("#login_help").show();
<?php
}
?>

function get_left_time(left_time){
	var secondTime = parseInt(left_time);
	var minuteTime = 0;
	var hourTime = 0;
	
	if(secondTime > 60) {
		minuteTime = parseInt(secondTime / 60);
		secondTime = parseInt(secondTime % 60);
		if(minuteTime > 60) {
			hourTime = parseInt(minuteTime / 60);
			minuteTime = parseInt(minuteTime % 60);
		}
	}
	var result = "" + parseInt(secondTime) + "<?php echo language("Second");?>";
	
	if(minuteTime > 0) {
		result = "" + parseInt(minuteTime) + "<?php echo language("Minute");?>" + result;
	}
	if(hourTime > 0) {
		result = "" + parseInt(hourTime) + "<?php echo language("Hour");?>" + result;
	}
	
	$("#left_time").html(result);
	left_time--;
	if(left_time>=0){
		setTimeout(get_left_time,1000,left_time);
	}else{
		$("#login_help").html("");
	}
}

$(function(){
	<?php if($dis_time){ ?>
	get_left_time(<?php echo $dis_time;?>);
	<?php } ?>
});
</script>

<?php
class Login{
	private $db;
	private $redis_cli;
	private $redis_password_info;
	private $redis_multi_info;
	
	function __construct(){
		$this->db = new Users();
		$this->redis_cli = new Predis\Client();

		// init data
		$this->redis_password_info = unserialize($this->redis_cli->get("login.password.times"));
		$this->redis_multi_info = unserialize($this->redis_cli->get("login.check.multiple"));
	}
	
	//Get the remaining time of locking the user
	public function get_dis_time(){
		if(!is_array($this->redis_password_info)){
			$this->redis_password_info = [];
		}

		return $this->check_lock_user_times($_POST['username']);
	}
	
	public function login(){
		global $language;
		
		$username = trim($_POST['username']);
		
		if($this->check_lock_user_times($username)){
			//用户已锁
			$_SESSION["error_report"] = language("User Lock","You entered too many errors and your account was temporarily locked. Please try again after the countdown is over.")."<span id='left_time'></span>";
			return false;
		}
		
		if($this->check_multi_sign_on($username)){
			//用户多点登录数已超过
			$_SESSION["error_report"] = language("User Limit","The number of user logins has exceeded the limit");
			return false;
		}
		
		if($username != ""){
			$password = trim($_POST['password']);
			
			$res = $this->db->get_one_user_by_username($username);
			$info = $res->fetchArray();
			
			$login_password = md5($password.'-'.$username);
			
			$time = time();
			
			if($login_password == $info['password']){//登录成功
				$ip = $this->db->get_client_ip();
				
				$_SESSION['id'] = $info['id'];
				$_SESSION['ip'] = $ip;
				$_SESSION['username'] = $username;
				$_SESSION['password'] = $login_password;
				setcookie(session_name(), session_id(), $time + 30*60, "/");
				
				//多点登录数据保存到redis
				$this->multi_sign_on_save_to_redis($username);
				
				//清空登录帐号输错密码的redis数据
				$this->clean_username_password_times($username);
				
				save_user_record($this->db,'Login');
				header("Location: /cgi-bin/php/system-status.php");
			}else if($this->is_exist_username($username)){
				
				//保存密码错误数到redis
				$this->passwd_error_save_to_redis($username);
				
				//密码错误
				$_SESSION["error_report"] = language("Password error");
			}else{
				//用户不存在
				$_SESSION["error_report"] = language("User does not exist");
			}
		}else{
			//用户不存在
			$_SESSION["error_report"] = language("User does not exist");
		}
	}
	
	private function clean_username_password_times($username){
		if(isset($this->redis_password_info[$username])){
			unset($this->redis_password_info[$username]);
		}
		
		$arr_str = serialize($this->redis_password_info);
		$this->redis_cli->set("login.password.times",$arr_str);
	}
	
	private function passwd_error_save_to_redis($username){
		$time = time();
		
		if(!isset($this->redis_password_info[$username][1]) || $this->redis_password_info[$username][1] == ""){
			$times = 1;
		}else{
			$times = $this->redis_password_info[$username][1];
			$times++;
		}
		
		$arr = [$username=>[$time,$times]];
		$arr += $this->redis_password_info;
		$arr_str = serialize($arr);
		$this->redis_cli->set("login.password.times",$arr_str);
	}
	
	private function multi_sign_on_save_to_redis($username){
		//redis key:login.check.multiple
		$time = time();
		$ip = $this->db->get_client_ip();
		if(isset($this->redis_multi_info[$username])){
			$has_ip = 0;
			for($i=0;$i<count($this->redis_multi_info[$username]);$i++){
				if($this->redis_multi_info[$username][$i][0] == $ip){
					$this->redis_multi_info[$username][$i][1] = $time;//存在ip就修改过期时间
					$has_ip = 1;
					break;
				}
			}
			
			if(!$has_ip){
				array_push($this->redis_multi_info[$username],[$ip,$time]);//不存在就添加ip和时间
			}
		}else{
			$this->redis_multi_info = [$username=>[[$ip,$time]]];//初始化redis数据
		}
		
		$arr_str = serialize($this->redis_multi_info);
		
		$this->redis_cli->set("login.check.multiple",$arr_str);
	}
	
	//multiple-sign-on
	//['username'=>[[ip,time],[ip,time],[ip,time]]]
	private function check_multi_sign_on($username){
		$ip = $this->db->get_client_ip();
		
		$res = $this->db->get_one_user_by_username($username);
		$info = $res->fetchArray();
		$ip_num = $info['ip_num'];
		
		if($ip_num == 0){//无限制
			return false;
		}
		
		$n = 1;
		if(isset($this->redis_multi_info[$username])){
			$count = count($this->redis_multi_info[$username]);
			for($i=0;$i<$count;$i++){
				if(intval($this->redis_multi_info[$username][$i][1])+30*60 < time()){
					unset($this->redis_multi_info[$username][$i]);//删除过期数据
				}
				
				if($this->redis_multi_info[$username][$i][0] == $ip){
					$n = 0;//如果ip已存在，则不加1
				}
			}
			$this->redis_multi_info[$username] = array_values($this->redis_multi_info[$username]);
			$arr_str = serialize($this->redis_multi_info);
			$this->redis_cli->set("login.check.multiple",$arr_str);
			
			if((count($this->redis_multi_info[$username])+$n) > $ip_num){
				return true;
			}
		}else{
			return false;
		}
		
		return false;
	}

	//['username'=>[time,times]]
	public function check_lock_user_times($username){
		
		$redis_info = $this->redis_password_info[$username];
		$redis_time = $redis_info[0];
		$time = time();
		
		//不是同一天的初始化密码错误数
		$time_temp = explode('-',date('Y-m-d',$time));
		$redis_time_temp = explode('-',date('Y-m-d',$redis_time));
		
		if($time > $redis_time){
			if(($time_temp[0] > $redis_time_temp[0]) || //year
				($time_temp[0] = $redis_time_temp[0] && $time_temp[1] > $redis_time_temp[1]) || //month
				($time_temp[0] = $redis_time_temp[0] && $time_temp[1] = $redis_time_temp[1] && $time_temp[2] > $redis_time_temp[2]) //day
			){
				unset($this->redis_password_info[$username]);
				
				$arr_str = serialize($this->redis_password_info);
				$this->redis_cli->set("login.password.times",$arr_str);
			}
		}
		
		if($redis_info[1] == 3){
			//1 minute
			$dis = $redis_info[0]+60 - $time;
			if($dis>0){
				return $dis;
			}else{
				return false;
			}
		}else if($redis_info[1] == 6){
			//10 minute
			$dis = $redis_info[0]+60*10 - $time;
			if($dis>0){
				return $dis;
			}else{
				return false;
			}
		}else if($redis_info[1] == 9){
			//30 minute
			$dis = $redis_info[0]+60*30 - $time;
			if($dis>0){
				return $dis;
			}else{
				return false;
			}
		}else if($redis_info[1] == 12){
			//60 minute
			$dis = $redis_info[0]+60*60 - $time;
			if($dis>0){
				return $dis;
			}else{
				return false;
			}
		}else if($redis_info[1] >= 15){
			//6 hours
			$dis = $redis_info[0]+60*60*6 - $time;
			if($dis>0){
				return $dis;
			}else{
				return false;
			}
		}
		
		return false;
	}
	
	public function user_already_login(){
		$session_id = $_SESSION['id'];
		if($session_id != ""){
			$session_username = $_SESSION['username'];
			$session_password = $_SESSION['password'];
			
			$res = $this->db->get_one_user_by_id($session_id);
			
			$info = $res->fetchArray();
			
			if($session_username == $info['username'] && $session_password == $info['password']){
				header("Location: /cgi-bin/php/system-status.php");
			}
		}
	}

	private function is_exist_username($username){
		$res = $this->db->get_all_username();
		while($info = $res->fetchArray()){
			if($username == $info['username']){
				return true;
			}
		}
		return false;
	}
}
?>