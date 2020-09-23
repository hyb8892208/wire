<!--The page of Change Password -->
<div class="ant-card-body" data-for="first-page" style="padding: 20px;">
  <div class="ant-row ant-form-item">
    <div class="ant-col-4 ant-form-item-label">
      <label for="new_username" class="" title=""><span><?php echo language("New Username");?>:</span></label>
    </div>
    <div class="ant-col-6 ant-form-item-control-wrapper">
      <div class="ant-form-item-control ">
        <!-- <input type="password" name="old_password" class="ant-input ant-input-lg hidden"> -->
        <input type="text" maxlength="32" value="" id="new_username" name="new_username" data-__meta="[object Object]" class="ant-input ant-input-lg">
      </div>
    </div>
  </div>
  <div class="ant-row ant-form-item">
    <div class="ant-col-4 ant-form-item-label">
      <label for="new_password" class="" title=""><span><?php echo language("New Password");?>:</span></label>
    </div>
    <div class="ant-col-6 ant-form-item-control-wrapper">
      <div class="ant-form-item-control ">
        <input type="password" value="" id="new_password" name="new_password" data-__meta="[object Object]" class="ant-input ant-input-lg">
      </div>
    </div>
  </div>
  <div class="ant-row ant-form-item">
    <div class="ant-col-4 ant-form-item-label">
      <label for="confirm_password" class="" title=""><span><?php echo language("Confirm Password");?>:</span></label>
    </div>
    <div class="ant-col-6 ant-form-item-control-wrapper">
      <div class="ant-form-item-control ">
        <input type="password" value="" id="confirm_password" name="confirm_password" data-__meta="[object Object]" class="ant-input ant-input-lg">
      </div>
    </div>
  </div>
  <div class="ant-row ant-form-item">
    <div class="content">
      <span style="color: rgb(170, 170, 170);font-weight: bold;"><?php echo language('The password must meet the following rules');?>:</span><br>
      <span style="color: rgb(170, 170, 170);">1. <?php echo language('At least eight characters.')?></span><br>
      <span style="color: rgb(170, 170, 170);">2. <?php echo language('At least one number.')?></span><br>
      <span style="color: rgb(170, 170, 170);">3. <?php echo language('At least one lowercase letter.')?></span><br>
      <span style="color: rgb(170, 170, 170);">4. <?php echo language('At least one uppercase letter.')?></span><br>
    </div>
  </div>
</div>
