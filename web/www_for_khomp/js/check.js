// Check valid ip addres or network address
// '<?php echo language('js check domain','Invalid domain or IP address.');?>'
function check_domain(str)
{
	var rex=/^(([a-z0-9](w|-){0,61}?[a-z0-9]|[a-z0-9]).){1,}(aero|arpa|asia|biz|cat|com|coop|co|edu|gov|info|int|jobs|mil|mobi|museum|name|net|org|pro|tel|travel|[a-z][a-z])(.[a-z][a-z]){0,1}$/i;

	if(rex.test(str)) {
		return true;
	}

	rex=/^((2[0-4][0-9]|25[0-5]|[01]?[0-9][0-9]?)\.){3}(2[0-4][0-9]|25[0-5]|[01]?[0-9][0-9]?)$/i;
	if(rex.test(str)) {
		return true;
	}

	return false;
}

// Check valid name (DIY)
// '<?php echo htmlentities(language('js check diyname','Allowed character must be any of [-_+.<>&0-9a-zA-Z],1 - 32 characters.'));?>'
function check_diyname(str)
{
	var rex=/^[-_+.<>&0-9a-zA-Z]{1,32}$/i;

	if(rex.test(str)) {
		return true;
	}

	return false;
}

//'<?php echo htmlentities(language('js check diypwd','Allowed character must be any of [0-9a-zA-Z`~!@$%^&*()_+{}|<>?-=[],./],4 - 32 characters.'));?>'
function check_diypwd(str)
{
	var rex=/^[0-9a-zA-Z`~!@$%^&*()_+\{\}|<>?\-=\[\]\,.\/]{4,32}$/i;

	if(rex.test(str)) {
		return true;
	}

	return false;
}

// Check valid GSM speacker volume level
// '<?php echo language('js check gsmvol',' Volume range: 0-100.');?>'
function check_gsmvol(str)
{
	if(parseInt(str) >= 0 && parseInt(str) <= 100) {
		return true;
	}

	return false;
}

// Check valid GSM microphone gain level
// '<?php echo language('js check gsmmic',' Volume range: 0-15.');?>'
function check_gsmmic(str)
{
	if(str >= 0 && str <= 15) {
		return true;
	}

	return false;
}

// Check txgain txdgain rxgain
function check_gsmgain(str)
{
	if(!isNaN(str) && parseInt(str) >= -1 && parseInt(str) <= 65535) {
		return true;
	}

	return false;
}

// Check send message
function check_send_msg(str)
{
	if(str.length >0 && str.length <= 128){
		return true;
	}
	return false;
}

// Check match key
function check_match_key(str)
{
	if(str.length >0 && str.length <= 32){
		return true;
	}
	return false;
}

// Check interval
function check_interval(str)
{
	if(!isNaN(str) && parseInt(str) >=0 && parseInt(str) <=30000){
		return true;
	}
	return false;
}

function check_call_counts_query(str)
{
	if(!isNaN(str) && parseInt(str) >=0 && parseInt(str) <=100){
		return true;
	}
	return false;
}

// Check valid ADC Chip Gain
// '<?php echo language('js check adcchipgain',' Gain range:  -42 - 20.');?>'
function check_adcchipgain(str)
{
	if(str >= -42 && str <= 20) {
		return true;
	}

	return false;
}

function check_adcgain(str)
{
	return check_adcchipgain(str);
}

function check_dacgain(str)
{
	return check_adcchipgain(str);
}

// '<?php echo language('js check dialprefix','Must be 0-9 or \\\'+\\\',\\\'*\\\',\\\'#\\\' ');?>'
function check_dialprefix(str)
{
	var rex=/^[0-9+\*#]{0,10}$/i;
	if(rex.test(str)) {
		return true;
	}

	return false;
}

// Check valid PIN of SIM card
// '<?php echo language('js check gsmpin','Must be 4 - 12 digits');?>'
function check_gsmpin(str)
{
	var rex=/^[0-9]{4,12}$/i;
	if(rex.test(str)) {
		return true;
	}

	return false;
}

// Check valid email address
// '<?php echo language('js check email','Please input a valid email address');?>'
function check_email(str)
{
	//var rex=/^([.a-zA-Z0-9_-])+@([a-zA-Z0-9_-])+((\.[a-zA-Z0-9_-]{2,3}){1,2})$/;
	var rex=/^([.a-zA-Z0-9_-])+@([a-zA-Z0-9_-])+((\.[a-zA-Z0-9_-]+)+)$/;

	if(rex.test(str)) {
		return true;
	}

	return false;
}

// '<?php echo htmlentities(language('js check sipendp','Allowed character must be any of [0-9a-zA-Z`~!@#$%^*()_{}:|?-=.], 1-32 characters.'));?>'
function check_sipendp(str)
{
	var rex=/^[0-9a-zA-Z`~!@#$%^*()_{}:|?\-=.]{1,32}$/i;

	if(rex.test(str)) {
		return true;
	}

	return false;
}

// '<?php echo language('js check sipname','Allowed character must be any of [0-9a-zA-Z~$*()-=_?.], length: 1-32');?>'
function check_sipname(str)
{
	var rex=/^[0-9a-zA-Z$*()\-=_.]{1,32}$/i;

	if(rex.test(str)) {
		return true;
	}

	return false;
}

// '<?php echo htmlentities(language('js check sippwd','Allowed character must be any of [0-9a-zA-Z`~!@#$%^&*()_+{}|<>?-=[],./],4 - 32 characters.'));?>'
function check_sippwd(str)
{
	var rex=/^[0-9a-zA-Z`~!#$%^&*()_+\{\}|<>\-=\,.]{1,32}$/i;

	if(rex.test(str)) {
		return true;
	}

	return false;
}

// '<?php echo htmlentities(language('js check routingname','Allowed character must be any of [0-9a-zA-Z`~!@#$%^*()_{}:|?-=.], 1-32 characters.'));?>'
function check_routingname(str)
{
	var rex=/^[0-9a-zA-Z`~!@#$%^*()_{}:|?\-=.]{1,32}$/i;

	if(rex.test(str)) {
		return true;
	}

	return false;
}

// Check valid network port
// '<?php echo language('js check networkport','Please input valid port number (1-65535)');?>'
function check_networkport(str)
{
	if(str >= 1 && str <= 65535) {
		return true;
	}

	return false;
}

// Check valid smtp user
// '<?php echo language('js check smtpuser','Please input a valid STMP user name');?>'
function check_smtpuser(str)
{
	var rex=/^[\w@.]{1,64}$/i;
	if(rex.test(str)) {
		return true;
	}

	return false;
}

// Check valid smtp password
// '<?php echo language('js check smtppwd','Password character range: 1-64');?>'
function check_smtppwd(str)
{
	var rex=/^[\w@.`~!#$%^&*()+=\-\{\}\\\|:;\'<>,.\?]{1,64}$/i;
	if(rex.test(str)) {
		return true;
	}

	return false;
}

function check_ntpserver(str)
{
	return check_domain(str);
}

// '<?php echo language('js check phonenum','Please input a valid phone number!');?>'
function check_phonenum(str)
{
	var rex=/^[0-9\+]{1,32}$/i;
	if(rex.test(str)) {
		return true;
	}

	return false;
}

// '<?php echo language('js check ussd','Please input a valid USSD number!');?>'
function check_ussd(str)
{
	var rex=/^[0-9\+\*\#]{1,32}$/i;
	if(rex.test(str)) {
		return true;
	}

	return false;
}

// '<?php echo language('js check ip','Please input a valid IP address');?>'
function check_ip(str)
{
	var rex=/^((2[0-4][0-9]|25[0-5]|[01]?[0-9][0-9]?)\.){3}(2[0-4][0-9]|25[0-5]|[01]?[0-9][0-9]?)$/i;
	if(rex.test(str)) {
		return true;
	}

	return false;
}

// '<?php echo language('js check mac','Please input a valid MAC address');?>'
function check_mac(str)
{
	var rex=/^([0-9a-fA-F]{2}:){5}([0-9a-fA-F]){2}$/i;
	if(rex.test(str)) {
		return true;
	}

	return false;
}

// '<?php echo language('js check pppoeuser','Please input valid username');?>'
function check_pppoeuser(str)
{
	var rex=/^[\w@.`~!#$%^&*()+=\-\{\}\\\|:;\'"<>,.\?]{1,64}$/i;
	if(rex.test(str)) {
		return true;
	}

	return false;
}

// '<?php echo language('js check pppoepwd','Please input valid password');?>'
function check_pppoepwd(str)
{
	var rex=/^[\w@.`~!#$%^&*()+=\-\{\}\\\|:;\'"<>,.\?]{1,64}$/i;
	if(rex.test(str)) {
		return true;
	}

	return false;
}

// '<?php echo language('js check float','Please input floating number');?>'
function check_float(str)
{
	var rex=/^\d+(.\d+)?$/i;
	if(rex.test(str)) {
		return true;
	}

	return false;
}

// '<?php echo language('js check integer','Please input integer number');?>'
function check_integer(str)
{
	var rex=/^\d+$/i;
	if(rex.test(str)) {
		return true;
	}

	return false;
}

function check_int32(str)
{
	var integer = parseInt(str);
	var rex=/^\d+$/i;
	if(rex.test(str) && integer<2147483648){
		return true;
	}
	return false;
}

function check_number(str)
{
	var rex=/^\d+$/i;
	if(rex.test(str)) {
		return true;
	}

	return false;
}

function check_datetime(str)
{
	/* Date Time Format: 2013-09-04 10:07:01 */
	var rex=/^(0?[1-9]|[1-2]\d|3[0-1])\/(0?[1-9]|1[0-2])\/\d{4} ([0-1]\d|2[0-3]):[0-5]\d:[0-5]\d$/i;
	if(rex.test(str)) {
		return true;
	}

	return false;
}

function con_str(str)
{
	return '<font color=ff0000>&nbsp;&nbsp;*'+str+'</font>';
}


function readfiles(file) 
{
	if(typeof window.ActiveXObject != 'undefined') {
		/*alert("en");
		var files = document.getElementById('number_book').files;
		var content = "";
		try {
			var fso = new ActiveXObject("Scripting.FileSystemObject"); 
			var name = $("#number_book").val();
			alert('"'+name+'"');
			var name = document.getElementById('number_book').value;
			alert(name);
			
			var reader = fso.openTextFile("F:\\3.ÏîÄ¿\\24.sms_sender\\phone_number_book_not_valid.txt", 1);
			while(!reader.AtEndofStream) {
				content += reader.readline();
				content += "\n";
			}       
			alert(content);
			reader.close(); // close the reader
		} catch (e) {
			alert("Internet Explore read local file error: \n" + e);
			return;
		}
		document.getElementById("dest_num").value=content;
		*/return;
	}else{
		var files = document.getElementById('number_book').files;
		if (!file.length) {
			alert('Please select a file!');
			return;
		}
		var file = files[0];

		var reader = new FileReader();

		// If we use onloadend, we need to check the readyState.
		reader.onloadend = function(evt) {
			if (evt.target.readyState == FileReader.DONE) { // DONE == 2
				document.getElementById('byte_content').textContent = evt.target.result;
				//document.getElementById('byte_range').textContent = ['Read bytes: ', ' 0 ', ' - ', ' file end ',' of ', file.size, ' byte file'].join('');
			}
		};

		//read file;
		reader.readAsBinaryString(file);
		reader.onload=function(e){
			document.getElementById("dest_num").value=this.result;
		}
		return;
	}
}
