<!--The page of SIP Endpoints -->
<div class="ant-card-body" data-for="four-page" style="padding: 20px;display:none">
	<div class="ant-row">
		<div class="ant-col-24">
			<div class="section-title"><span><?php echo language('SIP Endpoint');?></span></div>
		</div>
	</div>
	<div class="ant-row ant-form-item">
		<div class="ant-col-4 ant-form-item-label">
			<label for="sip_endpoint_name" class="" title=""><span><?php echo language("Name");?></span></label>
		</div>
		<div class="ant-col-6 ant-form-item-control-wrapper">
			<div class="ant-form-item-control has-success">
				<input type="text" value="" id="sip_endpoint_name" name="sip_endpoint_name" data-__meta="[object Object]" class="ant-input ant-input-lg">
			</div>
		</div>
	</div>
	<div class="ant-row ant-form-item">
		<div class="ant-col-4 ant-form-item-label">
			<label for="sip_endpoint_username" class="" title=""><span><?php echo language("User Name");?></span></label>
		</div>
		<div class="ant-col-6 ant-form-item-control-wrapper">
			<div class="ant-form-item-control has-success">
				<input type="text" value="" id="sip_endpoint_username" name="sip_endpoint_username" data-__meta="[object Object]" class="ant-input ant-input-lg">
			</div>
		</div>
	</div>
	<div class="ant-row ant-form-item">
		<div class="ant-col-4 ant-form-item-label">
			<label for="sip_endpoint_password" class="" title=""><span><?php echo language("Password");?></span></label>
		</div>
		<div class="ant-col-6 ant-form-item-control-wrapper">
			<div class="ant-form-item-control has-success">
				<input type="password" value="" id="sip_endpoint_password" name="sip_endpoint_password" data-__meta="[object Object]" class="ant-input ant-input-lg">
			</div>
		</div>
	</div>
	<div class="ant-row ant-form-item">
		<div class="ant-col-4 ant-form-item-label">
			<label for="registration" class="" title=""><span><?php echo language('Registration');?>:</span></label>
		</div>
		<div class="ant-col-6 ant-form-item-control-wrapper">
			<div class="ant-form-item-control ">
				<select class="ant-select-selection-selected-value" name="registration" id="registration" onchange="registrationChange()">
					<option value="none"    > <?php echo language('_None');?> </option>
					
					<?php
					session_start();
					if($_SESSION['id'] == 1){
					?>
					<option value= "server" > <?php echo language('Server');?> </option>
					<?php } ?>
					
					<option value= "client" > <?php echo language('Client');?> </option>
				</select>
			</div>
		</div>
	</div>
	<div class="ant-row ant-form-item">
		<div class="ant-col-4 ant-form-item-label">
			<label for="host" class="" title=""><span><?php echo language("Host Name or IP Address");?></span></label>
		</div>
		<div class="ant-col-6 ant-form-item-control-wrapper">
			<div class="ant-form-item-control has-success">
				<input type="text" value="" id="host" name="host" data-__meta="[object Object]" class="ant-input ant-input-lg">
			</div>
		</div>
	</div>
	<div class="ant-row ant-form-item hidden">
		<div class="ant-col-4 ant-form-item-label">
			<label for="transport" class="" title=""><span><?php echo language('Transport');?>:</span></label>
		</div>
		<div class="ant-col-6 ant-form-item-control-wrapper">
			<div class="ant-form-item-control ">
				<select class="ant-select-selection-selected-value" id="transport" name="transport">
					<option value="udp"  <?php echo $transport['udp']?> > UDP </option>
					<option value= "tcp" <?php echo $transport['tcp']?> > TCP </option>
				</select>
			</div>
		</div>
	</div>
	<div class="ant-row ant-form-item hidden">
		<div class="ant-col-4 ant-form-item-label">
			<label for="lan2_dns1" class="" title=""><span><?php echo language('NAT Traversal');?>:</span></label>
		</div>
		<div class="ant-col-6 ant-form-item-control-wrapper">
			<div class="ant-form-item-control ">
				<select class="ant-select-selection-selected-value" name="nat" id="nat">
					<option value= "no"> <?php echo language('_No');?> </option>
					<option value= "force_rport"> <?php echo language('Force report on');?> </option>
					<option value= "yes" > <?php echo language('_Yes');?> </option>
					<option value= "comedia"> <?php echo language('Report if requested and comedia');?> </option>
				</select>
			</div>
		</div>
	</div>
	<br>
	<div class="ant-row ant-form-item">
    <div class="content">
      <span style="color: rgb(170, 170, 170); font-weight: bold;"><?php echo language('Registration mode');?>:</span><br>
      <span style="color: rgb(170, 170, 170);">1. <?php echo language('None mode','None: Not Sending/Receiving SIP Regisration.')?></span><br>
      <!-- <span style="color: rgb(170, 170, 170);">2. <?php echo language('Server mode', 'Server: Acts as a SIP server, the gateway accepts registration from SIP Clients.')?></span><br> -->
      <span style="color: rgb(170, 170, 170);">2. <?php echo language('Client mode', 'Client: Acts as a SIP Client, the gateway sends registration to SIP servers.')?></span><br>
    </div>
  </div>
</div>
