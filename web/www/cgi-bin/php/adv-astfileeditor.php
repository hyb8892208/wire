<?php
require("/www/cgi-bin/inc/head.inc");
require("/www/cgi-bin/inc/menu.inc");
require_once("/www/cgi-bin/inc/function.inc");
?>

<script type="text/javascript" src="/js/functions.js">
</script>

<script type="text/javascript" src="/js/check.js">
</script>

<script type="text/javascript">
function goto()
{
	window.location.href="<?php echo get_self()?>";
}

function gotopage(page)
{
	if(event.keyCode == 13) {
		event.keyCode = 0;  //Must set
		event.which = 0;
		window.location.href="<?php echo get_self()."?&page="?>"+page;
	}
}

function goto_inputpage()
{
	window.location.href="<?php echo get_self()."?page="?>"+document.getElementById('page').value;
}

</script>

<?php
$dir = '/etc/asterisk';
function file_edit($filepath)
{
?>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<?php echo language('Edit');echo " ".basename($filepath);?>
	<br/>
	<textarea id="context" name="context" style="overflow-x:scroll; overflow-y:scroll;width:100%;height:450px;" ><?php readfile($filepath);?></textarea>
		<br/>
		<br/>
		<input type="hidden" name="send" id="send" value="" />
		<input type="hidden" name="filepath" value="<?php echo $filepath; ?>" />
		<input type="submit" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';"/> 
		<input type="button" value="<?php echo language('Cancel');?>" onclick="goto()" />
		<input type="submit" value="<?php echo language('Delete File');?>" 
			onclick="document.getElementById('send').value='Delete File';return confirm('<?php echo language('Delete File confirm','Are you sure to delete this file?');?>')" />
	</form>
<?php
}
?>

<?php
function new_edit($newfilename='')
{
?>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<?php echo language('Edit');?> <input type="text" name="newfilename" value="<?php echo $newfilename?>" style="width:120px;" />.conf
	<br/>
	<textarea id="context" name="context" style="overflow-x:scroll; overflow-y:scroll;width:100%;height:450px;" ></textarea>
		<br/>
		<br/>
		<input type="hidden" name="send" id="send" value="" />
		<input type="submit" value="<?php echo language('Save');?>" onclick="document.getElementById('send').value='Save';"/> 
		<input type="button" value="<?php echo language('Cancel');?>" onclick="goto()" />
	</form>
<?php
}
?>


<?php
function is_cfgfile($filepath,$exist = true)
{
	global $dir;

	if($dir == substr($filepath,0,strlen($dir))) {
		if('.conf' == substr($filepath,-5)) {
			if(dirname($filepath) === $dir) {
				if($exist) {
					if(file_exists($filepath)) {
						return true;
					}
				} else {
					return true;
				}
			}
		}
	}
	return false;
}

function write_to_file($filepath, $context)
{
	$context = str_replace("\r",'',$context);

	$hlock=lock_file($filepath);
	$fh = @fopen($filepath,"w");
	@fwrite($fh,$context);
	@fclose($fh);
	unlock_file($hlock);

	save_to_flash('/etc/asterisk','/etc/cfg');
}
?>


<?php
function show_files()
{
	if(isset($_GET['page']) && is_numeric($_GET['page']) && $_GET['page'] > 1 )
		$cur_page = $_GET['page'];
	else
		$cur_page = 1;

	global $dir;
	$all_files = array();
	$i = 0;
	$handle = opendir($dir);
	if($handle) {
		while(false !== ($file = readdir($handle))) {
			 if ($file != '.' && $file != '..') {
		$filepath = $dir . "/"  . $file;
				if (is_file($filepath)) {
					if(is_cfgfile($filepath)) {
						$all_files[$i] = $file;
						$i++;
					}
				}
			 }
		}

		if(count($all_files)) {
			sort($all_files);
		}

		closedir($handle);
	}

/*
	global $dir;
	$i = 0;
	exec("/bin/ls $dir",$list_files);
	foreach($list_files as $file) {
		if ($file != '.' && $file != '..') {
			$filepath = $dir . "/"  . $file;
			if (is_file($filepath)) {
				if(is_cfgfile($filepath)) {
					$all_files[$i] = $file;
					$i++;
				}
			}
		}
	}
*/

	$page_count = ceil($i/10);
	if($cur_page > $page_count) {
		$cur_page = $page_count;
	}

?>
	<form enctype="multipart/form-data" action="<?php echo get_self() ?>" method="post">
	<div id="tab">
		<li class="tb1">&nbsp;</li>
		<li class="tbg"><?php echo language('Configuration Files');?></li>
		<li class="tb2">&nbsp;</li>
	</div>
	<table width="100%" class="tshow" >
		<tr>
			<th>
				<?php echo language('File Name');?>
			</th>
			<th>
				<?php echo language('File Size');?>
			</th>
		</tr>
<?php
	for($i=($cur_page-1)*10; $i<($cur_page-1)*10+10; $i++) {
		if(!isset($all_files[$i])) {
			break;
		}

		echo "<tr>";
		echo "<td><a href=\"" . get_self() . "?file=".$all_files[$i]."\">".$all_files[$i]."</a></td>";
		echo "<td>".filesize($dir."/".$all_files[$i])."</td>";
		echo "</tr>";
	}

?>
	</table>

<?php
	if($page_count >= 2) {
		echo '<br/>';
		echo '<div class="pg">';

		if( $cur_page > 1 ) {
			$page = $cur_page - 1;
			echo "<a title=\"";echo language('Previous page');echo "\" href=\"".get_self()."?page=$page\" class=\"prev\"></a>";
		} else {
			
		}
			
		if($cur_page-5 > 1) {
			$s = $cur_page-5;
		} else {
			$s = 1;
		}

		if($s + 10 < $page_count) {
			$e = $s + 10;
		} else {
			$e = $page_count;
		}

		for($i = $s; $i <= $e; $i++) {
			if($i != $cur_page) {
				echo "<a href=\"".get_self()."?page=$i\" >$i</a>";
			} else {
				echo "<strong>$cur_page</strong>";
			}
		}

		if( $cur_page < $page_count ) {
			$page = $cur_page + 1;
			echo "<a title=\"";echo language('Next page');echo "\" href=\"".get_self()."?&page=$page\" class=\"nxt\" ></a>";
		} else {
			
		}

		echo "<label>";
		echo "<input type=\"text\" id=\"page\" name=\"page\" value=\"$cur_page\" size=\"2\" class=\"px\" title=\"";
		echo language('input pages help','Please input your page number, and press [Enter] to skip to.');echo "\" onkeypress=\"gotopage(this.value)\" >";
		echo "<span title=\"";echo language('total pages');echo ": $page_count\"> / $page_count</span>";
		echo "</label>";
?>
		<a title="<?php echo language('goto input page');?>" style="cursor:pointer;" onclick="goto_inputpage()"><?php echo language('go');?></a>
		</div>
<?php
	}
?>
	<div id="newline"></div>
	<br/>
	<input type="hidden" name="send" id="send" value="" />
	<input type="submit" value="<?php echo language('New Configuration File');?>" onclick="document.getElementById('send').value='New Configuration File';"/>
	<input type="submit" value="<?php echo language('Reload Asterisk');?>" 
		onclick="document.getElementById('send').value='Reload Asterisk';return confirm('<?php echo language('Reload Asterisk confirm','Are you sure to reload asterisk now?');?>')" />
	</form>

	<script type="text/javascript">
	document.getElementsByTagName('form')[0].onkeypress = function(e) {
		var e = e || event;
		var keyNum = e.which || e.keyCode;
		return keyNum == 13 ? false : true;
	};
	</script>
<?php
}
?>

<?php
$edit = false;
if($_GET) {
	if(isset($_GET['file'])) {
		$file_name = trim($_GET['file']);
		$file_name = $dir."/".$file_name;
		if(is_cfgfile($file_name)) {
			file_edit($file_name);
			$edit = true;
		}
	}
}

if($_POST && isset($_POST['send'])) {
	if($_POST['send'] == 'Save') {
		if(isset($_POST['context'])) {
			if(isset($_POST['filepath'])) {
				if(is_cfgfile($_POST['filepath'])) {
					write_to_file($_POST['filepath'],$_POST['context']);
				}
			} else if(isset($_POST['newfilename'])) {
				$newfilename = trim($_POST['newfilename']);
				$full_filepath = $dir.'/'.$newfilename.'.conf';
				if($newfilename == '') {
					new_edit();
					$edit = true;
					echo language('Save file name',"Please in put file name!");
				} else if(file_exists($full_filepath)) {
					new_edit($newfilename);
					$edit = true;
					echo language('Save file exist',"File already exist!");
				} else {
					if(is_cfgfile($full_filepath,false)) {
						write_to_file($full_filepath,$_POST['context']);
					}
				}
			}
		}
	} else if($_POST['send'] == 'Reload Asterisk') {
		ast_reload();
	} else if($_POST['send'] == 'New Configuration File') {
		new_edit();
		$edit = true;
	} else if($_POST['send'] == 'Delete File') {
		if(isset($_POST['filepath'])) {
			if(is_cfgfile($_POST['filepath'])) {
				unlink($_POST['filepath']);
			}
		} 
	}
}

if(!$edit) {
	show_files();
}

?>

<?php require("/www/cgi-bin/inc/boot.inc");?>
