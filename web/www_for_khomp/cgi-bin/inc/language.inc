<?php

include_once("/www/cgi-bin/inc/function.inc");

/* 

conf_path : /etc/asterisk/gw/web_language.conf
------------------------
[general]
language=chinese

[list]
chinese=中文
portuguese=Português
-----------------------


language package : /etc/asterisk/gw/web_language/xxx
-----------------------
language#chinese#简体中文
...
-----------------------
Notice the first line format.

*/

/********************************************************************
 *
 *      name : web_language_init()
 *
 *      function : According to /etc/asterisk/gw/web_language.conf,
 *                 cache /etc/asterisk/gw/web_language/xxx 
 *                 to /tmp/language.cache.
 *
 ********************************************************************/

function web_language_init()
{
	/*
	 * get available language:
	 * 	1.'/www/lang': readonly;
	 * 	2.'/etc/asterisk/gw/web_language': editable.
	 */
	$language_dir_array = array('/www/lang','/etc/asterisk/gw/web_language');
	$language_list = '';
	foreach($language_dir_array as $language_dir){
		if(is_dir($language_dir)){
			$dir_handle = opendir($language_dir);
			while(($file = readdir($dir_handle))!=false){
				if($file == '.' || $file == '..')continue;
				$info = '';
				if(is_language("$language_dir/$file",$info)){
					$language_list[$info['key']] = $info['value'];
				}else{
					continue;
				}
			}
			closedir($dir_handle);
		}
	}

	/* update web_language.conf */
	$conf_path = "/etc/asterisk/gw/web_language.conf";
	$conf_array = get_conf($conf_path);
	if(isset($conf_array['general']['language']) && $conf_array['general']['language'] != "")
		$language_type =$conf_array['general']['language'];
	else
		$language_type = "english";//default language
	if(isset($conf_array['list']) && $conf_array['list'] === $language_list && isset($conf_array['general']['language']) && $conf_array['general']['language'] != ""){
		echo '';//No change, do not edit conf file.
	}else{
		if(isset($conf_array['general']['language']) && $conf_array['general']['language'] != "")$language_type =$conf_array['general']['language'];
		$language_conf  = "[general]\n";
		$language_conf .= "language=$language_type\n\n";
		$language_conf .= "[list]\n";
		if(is_array($language_list)){
			foreach($language_list as $key => $value){
				$language_conf .= "$key=$value\n";
			}
		}

		if(!file_exists($conf_path)) {
			fclose(fopen($conf_path,"w"));
		}

		$hlock = lock_file($conf_path);
		file_put_contents($conf_path, $language_conf); 
		unlock_file($hlock);
	}

	/* set cache */
	if(is_file('/www/lang/'.$language_type)){
		$language_file = '/www/lang/'.$language_type;
	}else if(is_file('/etc/asterisk/gw/web_language/'.$language_type)){
		$language_file = '/etc/asterisk/gw/web_language/'.$language_type;
	}else{
		$language_file = '/www/lang/english';
	}
	$cache_file = "/tmp/web/language.cache";
	set_web_language_cache($cache_file, $language_file);

	return true;
}

function is_language($path,&$info)
{
	$fh = fopen($path,'r');
	$head = stream_get_line($fh,1024,"\n");
	fclose($fh);

	$head_array = explode('#',$head);
	if(isset($head_array[2]) && 'language'==trim($head_array[0]) ){
		$language_key = trim($head_array[1]);
		$language_value = trim($head_array[2]);
		if(!preg_match('/^[a-zA-Z]+$/',$language_key) || $language_value ==''){
			return false;
		}else{
			$info['key']=$language_key;
			$info['value']=$language_value;
			return true;
		}
	}else{
		return false;
	}
}

function set_web_language_cache($cache_file, $language_path) 
{
	/* cache language file */
	if(is_file($language_path)){
		$language_array = parse_ini_file($language_path, true);//two-dimensional array
		//$language_array = parse_ini_file($language_path, false);//one-dimensional array
		$flock = lock_file($cache_file); 
		fclose(fopen($cache_file,"w")); 
		file_put_contents($cache_file,serialize($language_array)); 
		unlock_file($flock);
	}
	return true;
}

function get_web_language_cache($cache_file)
{
	if(!file_exists($cache_file)) {
		return false;
	}

	$handle = fopen($cache_file, "r");
	$language_array = unserialize(fread($handle, filesize ($cache_file)));
	fclose($handle); 

	return $language_array;
}

function language($key, $default=NULL)
{
	global $language;
	global $language_debug;
	
	if($language_debug == 'true'){
		return '['.$key.']';
	}
	
	$page_path = get_self();
	$lang = [];
	foreach($language as $k => $v){
		if(strstr($page_path, $k)){
			$lang = array_merge($lang,$language[$k]);
		}
		
		if(!strstr($k, '-') && !strstr($k, '_')){
			$lang = array_merge($lang,$language[$k]);
			
		}
		
	}
	
	if(isset($lang[$key]) && $lang[$key]!=""){
		return $lang[$key];
	}else{
		if($default != ""){
			return $default;
		}else{
			switch($key){
			case '_None':return 'None';
			case '_Yes':return 'Yes';
			case '_No':return 'No';
			case '_On':return 'On';
			default:return $key;
			}
		}
	}
}

function is_show_language_help($key){
	global $language;
	
	$page_path = get_self();
	$lang = [];
	foreach($language as $k => $v){
		if(strstr($page_path, $k)){
			$lang = array_merge($lang,$language[$k]);
		}
		
		if(!strstr($k, '-') && !strstr($k, '_')){
			$lang = array_merge($lang,$language[$k]);
			
		}
	}
	
	if(isset($lang[$key." help"]) && $lang[$key." help"]!=""){
		return true;
	}else{
		return false;
	}
}

?>
