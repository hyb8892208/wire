<?php

include_once("/www/cgi-bin/inc/function.inc");

function get_config($config_file, $section=NULL)
{
	$config_array = array();
	$new_section = "";
	$section_find = false;

	if(!file_exists($config_file))
		return false;

	$flock = lock_file($config_file);
	$fh = fopen($config_file, 'r');
	while(!feof($fh)){
		$line = trim(stream_get_line($fh, 1024, "\n"));
		if((strncmp($line, "#", 1)==0) || (strncmp($line, ";", 1)==0)){
			continue;
		}else if(strncmp($line, "[", 1)==0){
			if(($pos=strpos($line, "]"))>1){
				$new_section = trim(substr($line,1,$pos-1));
				if($section != NULL){
					if($section_find){
						break;
					}else if(strcmp($new_section,$section) == 0){
						$section_find = true;
					}else{
						$new_section = "";
					}
				}
			}
			continue;
		}else if($new_section != ""){
			$line_array = explode("=", $line, 2);
			if(isset($line_array[1])){
				$key = trim($line_array[0]);
				$value = trim($line_array[1]);
				if($key!=""){
					if(!isset($config_array[$new_section][$key])){
						$config_array[$new_section][$key] = $value;
					}else{
						for($i=1;isset($config_array[$new_section][$key."[$i]"]);$i++){
						}
						$config_array[$new_section][$key."[$i]"] = $value;
					}
				}
			}
		}
	}
	fclose($fh);
	unlock_file($flock);	
	return $config_array;
}

function set_config($config_file, $config_array)
{
	$contents = "";
	if(is_array($config_array)){
		foreach($config_array as $section_key => $section_value){
			$contents .= "\n[$section_key]\n";
			if(is_array($config_array[$section_key])){
				foreach($config_array[$section_key] as $key => $value){
					$contents .= "$key=$value\n";
				}
			}
		}
	}

	$flock = lock_file($config_file);
	file_put_contents($config_file, $contents);
	unlock_file($config_file);
	return true;
}

function del_config_section($config_file, $section)
{
	$contents = "";
	$contents_find = true;

	$flock = lock_file($config_file);
	$fh = fopen($config_file, 'r+');
	while(!feof($fh)){
		$line = trim(stream_get_line($fh, 1024, "\n"));
		if(strncmp($line, "[", 1)==0){
			if(strstr($line, "]")){
				$new_section = trim(substr($line,1,strpos($line,"]")-1));
				if(strcmp($new_section,$section) == 0){
					$contents_find = false;
				}else{
					$contents_find = true;
				}
			}
		}
		if($contents_find){
			$contents .= "$line\n";
		}
	}
	//fclose($fh);
	//file_put_contents($config_file, $contents);
	ftruncate($fh, 0);
	fwrite($fh, $contents);
	fclose($fh);
	unlock_file($flock);	
	return true;
}

function set_config_section($config_file, $section, $config_array)
{
	if(!file_exists($config_file))
		return false;

	$section_contents = "\n[$section]\n";
	if(is_array($config_array[$section])){
		foreach($config_array[$section] as $key => $value){
			$section_contents .= "$key=$value\n";
		}
	}
	
	$contents = "";
	$contents_find = true;
	$new_section = "";
	$section_find = false;

	$flock = lock_file($config_file);
	$fh = fopen($config_file, 'r+');
	while(!feof($fh)){
		$line = trim(stream_get_line($fh, 1024, "\n"));
		if(strncmp($line, "[", 1)==0){
			if(($pos=strstr($line, "]"))>1){
				$new_section = trim(substr($line,1,$pos-1));
				if(strcmp($section,$new_section) == 0){
					$section_find = true;
					$contents .= $section_contents;
					$contents_find = false;
				}else{
					$contents_find = true;
				}
			}
		}
		if($contents_find){
			$contents .= "$line\n";
		}
	}
	if(!$section_find)
		$contents .= $section_contents;
	//fclose($fh);
	//file_put_contents($config_file, $contents);
	ftruncate($fh, 0);
	fwrite($fh, $contents);
	fclose($fh);
	unlock_file($flock);	
	return true;	
}

function del_config_key($config_file, $section, $key)
{
	if(!file_exists($config_file))
		return false;

	$contents = "";
	$contents_find = true;
	$new_section = "";
	$section_find = false;

	$flock = lock_file($config_file);
	$fh = fopen($config_file, 'r+');
	while(!feof($fh)){
		$line = trim(stream_get_line($fh, 1024, "\n"));
		if(strncmp($line, "[", 1)==0){
			if(($pos=strpos($line, "]"))>1){
				$new_section = trim(substr($line,1,$pos-1));
				if(strcmp($new_section,$section) == 0){
					$section_find = true;
				}else{
					$section_find = false;
				}
			}
		}
		if($section_find){
			$line_array = explode("=", $line, 2);
			if(isset($line_array[1])){
				$new_key = trim($line_array[0]);
				if(strcmp($new_key,$key) == 0){
					$contents_find = false;
				}else{
					$contents_find = true;
				}
			}
		}
		if($contents_find)
			$contents .= "$line\n";
	}
	//fclose($fh);
	//file_put_contents($config_file, $contents);
	ftruncate($fh, 0);
	fwrite($fh, $contents);
	fclose($fh);
	unlock_file($flock);	
	return true;	
}

function set_config_key($config_file, $section, $key, $value)
{
	if(!file_exists($config_file))
		return false;

	$contents = "";
	$contents_find = true;
	$new_section = "";
	$section_find = false;
	$new_key = "";
	$key_find = false;

	$flock = lock_file($config_file);
	$fh = fopen($config_file, 'r+');
	while(!feof($fh)){
		$line = trim(stream_get_line($fh, 1024, "\n"));
		if(strncmp($line, "[", 1)==0){
			if(($pos=strpos($line, "]"))>1){
				$new_section = trim(substr($line,1,$pos-1));
				if(strcmp($new_section,$section) == 0){
					$section_find = true;
				}else{
					$section_find = false;
					$contents_find = true;
				}
			}
		}
		if($section_find){
			$line_array = explode("=", $line, 2);
			if(isset($line_array[1])){
				$new_key = trim($line_array[0]);
				if(strcmp($new_key,$key) == 0){
					$key_find = true;
					$contents .= "$key=$value\n";
					$contents_find = false;
				}else{
					$contents_find = true;
				}
			}
		}
		if($contents_find)
			$contents .= "$line\n";
	}
	if(!$key_find){
		$contents .= "\n[$section]\n";
		$contents .= "$key=$value\n";
	}
	//fclose($fh);
	//file_put_contents($config_file, $contents);
	ftruncate($fh, 0);
	fwrite($fh, $contents);
	fclose($fh);
	unlock_file($flock);	
	return true;	
}


?>
