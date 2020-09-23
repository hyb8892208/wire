<?php

// Check valid ip addres or network address
function check_domain($str)
{
	if(preg_match('/(([a-z0-9](w|-){0,61}?[a-z0-9]|[a-z0-9]).){1,}(aero|arpa|asia|biz|cat|com|coop|co|edu|gov|info|int|jobs|mil|mobi|museum|name|net|org|pro|tel|travel|[a-z][a-z])(.[a-z][a-z]){0,1}/', $str)) {
		return false;
	}

	return 'Invalid domain or IP address.';
}

// Check valid name (DIY)
// -_+.1-9a-zA-Z, 32 character long
function check_name($str)
{
	if(preg_match('/[-_+.1-9a-zA-Z]{4,32}/',$str)) {
		return false;
	}

	return 'Allowed character must be any of [-_+.1-9a-zA-Z],4 - 32 characters.';
}


// Check valid GSM speacker volume level
function check_gsmvol($str)
{
	if($str >= 0 && $str <= 100) {
		return false;
	}

	return ' Volume range: 0-100.';
}

// Check valid GSM microphone gain level
function check_gsmmic($str)
{
	if($str >= 0 && $str <= 15) {
		return false;
	}

	return ' Volume range: 0-15.';
}

// Check valid ADC Chip Gain
function check_adcchipgain($str)
{
	if($str >= -42 && $str <= 20) {
		return false;
	}

	return ' Gain range:  -42 - 20.';
}

function check_adcgain($str)
{
	return check_adcchipgain($str);
}

function check_dacgain($str)
{
	return check_adcchipgain($str);
}

// Check valid PIN of SIM card
function check_gsmpin($str)
{
	if(preg_match('/[0-9]{4,12}/',$str)) {
		return false;
	}

	return 'Must be 4 - 12 digits';
}

// Check valid email address
function check_email($str)
{
	if(preg_match('/^[a-z0-9_\-]+(\.[_a-z0-9\-]+)*@([_a-z0-9\-]+\.)+([a-z]{2}|aero|arpa|biz|com|coop|edu|gov|info|int|jobs|mil|museum|name|nato|net|org|pro|travel)$/',$str)) {
		return false;
	}

	return 'Please input a valid email address';
}

// Check valid network port
function check_networkport($str)
{
	if($str >= 1 && $str <= 65535) {
		return false;
	}

	return 'Please input valid port number';
}

// Check valid smtp user
function check_smtpuser($str)
{
	if(preg_match('/^[\w@.]{1,64}$/',$str)) {
		return false;
	}

	return 'Please input a valid STMP user name';
}

// Check valid smtp password
function check_smtppwd($str)
{
	if(preg_match('/^[\w@.`~!#$%^&*()+=\-\{\}\\\|:;\'"<>,.\?/]{1,64}$/',$str)) {
		return false;
	}

	return 'Password character range: 1-64';
}

function check_ntpserver($str)
{
	return check_domain($str);
}


function check_pwd($str)
{
	if(preg_match('/^[\w]{1,16}$/',$str)) {
		return false;
	}

	return 'Please input a valid password';
}

function check_phonenum($str)
{
	if(preg_match('/^[0-9\+]{1,32}$/',$str)) {
		return false;
	}

	return 'Please input a valid phone number!';
}

function check_ussd($str)
{
	if(preg_match('/^[0-9\+\*\#]{1,32}$/',$str)) {
		return false;
	}

	return 'Please input a valid USSD number!';
}

function check_ip($str)
{
	if(preg_match('/^((2[0-4][0-9]|25[0-5]|[01]?[0-9][0-9]?)\.){3}(2[0-4][0-9]|25[0-5]|[01]?[0-9][0-9]?)$/',$str)) {
		return false;
	}

	return 'Please input a valid IP address';
}

function check_mac($str)
{
	if(preg_match('/^([0-9a-fA-F]{2}:){5}([0-9a-fA-F]){2}$/',$str)) {
		return false;
	}

	return 'Please input a valid MAC address';
}

function check_pppoeuser($str)
{
	if(preg_match('/^[\w@.`~!#$%^&*()+=\-\{\}\\\|:;\'"<>,.\?/]{1,64}$/',$str)) {
		return false;
	}

	return 'Please input valid username';
}

function check_pppoepwd($str)
{
	if(preg_match('/^[\w@.`~!#$%^&*()+=\-\{\}\\\|:;\'"<>,.\?/]{1,64}$/',$str)) {
		return false;
	}

	return 'Please input valid password';
}

function output_str($str)
{

}

?>
