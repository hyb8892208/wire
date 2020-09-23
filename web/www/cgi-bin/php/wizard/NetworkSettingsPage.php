<!--The page of  -->

<div class="ant-card-body " data-for="third-page" style="padding: 20px;display:none">
  <div class="ant-row">
    <div class="ant-col-24">
      <div class="section-title"><span>LAN</span></div>
    </div>
  </div>
  <div class="ant-row ant-form-item">
    <div class="ant-col-4 ant-form-item-label">
      <label for="lan2_ip_method" class="" title=""><span><?php echo language('IP Method');?></span></label>
    </div>
    <div class="ant-col-6 ant-form-item-control-wrapper">
      <div class="ant-form-item-control has-success">
        <select id="lan_type" name="lan_type" onchange="LanTypeChange()" class="ant-select-selection-selected-value">
          <option  value="factory" <?php echo $type['factory'];?> ><?php echo language('Factory');?></option>
          <option  value="static" <?php echo $type['static'];?> ><?php echo language('Static');?></option>
          <option  value="dhcp" <?php echo $type['dhcp'];?> ><?php echo language('DHCP');?></option>
        </select>
      </div>
    </div>
  </div>
  
  <div class="ant-row ant-form-item">
    <div class="ant-col-4 ant-form-item-label">
      <label for="lan2_ip" class="" title=""><span><?php echo language('IP Address');?></span></label>
    </div>
    <div class="ant-col-6 ant-form-item-control-wrapper">
      <div class="ant-form-item-control has-success">
        <input type="text" value="" id="lan_ip_address" name="lan_ip_address" data-__meta="[object Object]" class="ant-input ant-input-lg">
      </div>
    </div>
  </div>
  <div class="ant-row ant-form-item">
    <div class="ant-col-4 ant-form-item-label">
      <label for="lan2_mask" class="" title=""><span><?php echo language('Subnet Mask');?></span></label>
    </div>
    <div class="ant-col-6 ant-form-item-control-wrapper">
      <div class="ant-form-item-control has-success">
        <input type="text" value="" id="lan_netmask" name="lan_netmask" data-__meta="[object Object]" class="ant-input ant-input-lg">
      </div>
    </div>
  </div>
  <div class="ant-row ant-form-item">
    <div class="ant-col-4 ant-form-item-label">
      <label for="lan2_gateway" class="" title=""><span><?php echo language('Gateway IP');?></span></label>
    </div>
    <div class="ant-col-6 ant-form-item-control-wrapper">
      <div class="ant-form-item-control has-success">
        <input type="text" value="" id="lan_gateway" name="lan_gateway" data-__meta="[object Object]" class="ant-input ant-input-lg">
      </div>
    </div>
  </div>
  <div id="lan_dns_setting">
    <div class="ant-row ant-form-item">
      <div class="ant-col-4 ant-form-item-label">
        <label for="lan2_dns1" class="" title=""><span><?php echo language('DNS Server 1');?></span></label>
      </div>
      <div class="ant-col-6 ant-form-item-control-wrapper">
        <div class="ant-form-item-control ">
          <input type="text" value="" id="lan_dns1" name="lan_dns1" data-__meta="[object Object]" class="ant-input ant-input-lg">
        </div>
      </div>
    </div>
    <div class="ant-row ant-form-item">
      <div class="ant-col-4 ant-form-item-label">
        <label for="lan2_dns1" class="" title=""><span><?php echo language('DNS Server 2');?></span></label>
      </div>
      <div class="ant-col-6 ant-form-item-control-wrapper">
        <div class="ant-form-item-control ">
          <input type="text" value="" id="lan_dns2" name="lan_dns2" data-__meta="[object Object]" class="ant-input ant-input-lg">
        </div>
      </div>
    </div>
    <div class="ant-row ant-form-item">
      <div class="ant-col-4 ant-form-item-label">
        <label for="lan2_dns1" class="" title=""><span><?php echo language('DNS Server 3');?></span></label>
      </div>
      <div class="ant-col-6 ant-form-item-control-wrapper">
        <div class="ant-form-item-control ">
          <input type="text" value="" id="lan_dns3" name="lan_dns3" data-__meta="[object Object]" class="ant-input ant-input-lg">
        </div>
      </div>
    </div>
  </div>
  
  
  <div class="ant-row">
    <div class="ant-col-24">
      <div class="section-title"><span><?php echo language('WAN');?></span></div>
    </div>
  </div>
  <div class="ant-row ant-form-item">
    <div class="ant-col-4 ant-form-item-label">
      <label for="lan2_ip_method" class="" title=""><span><?php echo language('IP Address');?></span></label>
    </div>
    <div class="ant-col-6 ant-form-item-control-wrapper">
      <div class="ant-form-item-control has-success">
        <select id="wan_type" name="wan_type" onchange="WanTypeChange()" class="ant-select-selection-selected-value ">
          <option  value="dhcp" <?php echo $type['dhcp'];?> ><?php echo language('DHCP');?></option>
          <!-- <option  value="factory" <?php echo $type['factory'];?> ><?php echo language('Factory');?></option> -->
          <option  value="static" <?php echo $type['static'];?> ><?php echo language('Static');?></option>
          <option  value="disable" <?php echo $type['dhcp'];?> ><?php echo language('Disable');?></option>
        </select>
      </div>
    </div>
  </div>
  <div id="wan_ipv4_setting" >
    <div class="ant-row ant-form-item">
      <div class="ant-col-4 ant-form-item-label">
        <label for="lan2_ip" class="" title=""><span><?php echo language('IP Address');?></span></label>
      </div>
      <div class="ant-col-6 ant-form-item-control-wrapper">
        <div class="ant-form-item-control has-success">
          <input type="text" value="" id="wan_ip_address" name="wan_ip_address" data-__meta="[object Object]" class="ant-input ant-input-lg">
        </div>
      </div>
    </div>
    <div class="ant-row ant-form-item">
      <div class="ant-col-4 ant-form-item-label">
        <label for="lan2_mask" class="" title=""><span><?php echo language('Subnet Mask');?></span></label>
      </div>
      <div class="ant-col-6 ant-form-item-control-wrapper">
        <div class="ant-form-item-control has-success">
          <input type="text" value="" id="wan_netmask" name="wan_netmask" data-__meta="[object Object]" class="ant-input ant-input-lg">
        </div>
      </div>
    </div>
    <div class="ant-row ant-form-item">
      <div class="ant-col-4 ant-form-item-label">
        <label for="lan2_gateway" class="" title=""><span><?php echo language('Gateway IP');?></span></label>
      </div>
      <div class="ant-col-6 ant-form-item-control-wrapper">
        <div class="ant-form-item-control has-success">
          <input type="text" value="" id="wan_gateway" name="wan_gateway" data-__meta="[object Object]" class="ant-input ant-input-lg">
        </div>
      </div>
    </div>
  </div>
</div>

