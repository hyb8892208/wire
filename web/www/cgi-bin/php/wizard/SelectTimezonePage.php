<!--The page of Select Timezone  -->
<div class="ant-card-body" data-for="second-page" style="padding: 20px;display:none">
	<div class="ant-row ant-form-item">
		<div class="ant-col-4 ant-form-item-label">
			<label for="lan2_ip_method" class="" title=""><span><?php echo language("Time Zone");?></span></label>
		</div>
		<div class="ant-col-6 ant-form-item-control-wrapper">
			<div class="ant-form-item-control has-success">
				<select id="system_timezone" name="system_timezone" onchange="modeChange(this)" class="ant-select-selection-selected-value">
					<optgroup label="<?php echo language('Australia');?>">
						<option  value="Australia/Melbourne@EST-10EST,M10.5.0,M3.5.0/3"><?php echo language('Melbourne,Canberra,Sydney');?></option>
						<option  value="Australia/Perth@WST-8"><?php echo language('Perth');?></option>
						<option  value="Australia/Brisbane@EST-10"><?php echo language('Brisbane');?></option>
						<option  value="Australia/Adelaide@CST-9:30CST,M10.5.0,M3.5.0/3"><?php echo language('Adelaide');?></option>
						<option  value="Australia/Darwin@CST-9:30"><?php echo language('Darwin');?></option>
						<option  value="Australia/Hobart@EST-10EST,M10.1.0,M3.5.0/3"><?php echo language('Hobart');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Europe');?>">
						<option  value="Europe/Amsterdam@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Amsterdam,Netherlands');?></option>
						<option  value="Europe/Athens@EET-2EEST,M3.5.0/3,M10.5.0/4"><?php echo language('Athens,Greece');?></option>
						<option  value="Europe/Berlin@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Berlin,Germany');?></option>
						<option  value="Europe/Brussels@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Brussels,Belgium');?></option>
						<option  value="Europe/Bratislava@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Bratislava,Slovakia');?></option>
						<option  value="Europe/Budapest@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Budapest,Hungary');?></option>
						<option  value="Europe/Copenhagen@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Copenhagen,Denmark');?></option>
						<option  value="Europe/Dublin@GMT0IST,M3.5.0/1,M10.5.0"><?php echo language('Dublin,Ireland');?></option>
						<option  value="Europe/Helsinki@EET-2EEST,M3.5.0/3,M10.5.0/4"><?php echo language('Helsinki,Finland');?></option>
						<option  value="Europe/Kiev@EET-2EEST,M3.5.0/3,M10.5.0/4"><?php echo language('Kyiv,Ukraine');?></option>
						<option  value="Europe/Lisbon@WET0WEST,M3.5.0/1,M10.5.0"><?php echo language('Lisbon,Portugal');?></option>
						<option  value="Europe/London@GMT0BST,M3.5.0/1,M10.5.0"><?php echo language('London,GreatBritain');?></option>
						<option  value="Europe/Madrid@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Madrid,Spain');?></option>
						<option  value="Europe/Oslo@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Oslo,Norway');?></option>
						<option  value="Europe/Paris@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Paris,France');?></option>
						<option  value="Europe/Prague@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Prague,CzechRepublic');?></option>
						<option  value="Europe/Rome@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Roma,Italy');?></option>
						<option  value="Europe/Moscow@MSK-3"><?php echo language('Moscow,Russia');?></option>
						<option  value="Europe/Stockholm@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Stockholm,Sweden');?></option>
						<option  value="Europe/Zurich@CET-1CEST,M3.5.0,M10.5.0/3"><?php echo language('Zurich,Switzerland');?></option>
					</optgroup>
					<optgroup label="<?php echo language('New Zealand');?>">
						<option  value="Pacific/Auckland@NZST-12NZDT,M10.1.0,M3.3.0/3"><?php echo language('Auckland, Wellington');?></option>
					</optgroup>
					<optgroup label="<?php echo language('USA and Canada');?>">
						<option  value="Pacific/Honolulu@HST10"><?php echo language('Hawaii Time');?></option>
						<option  value="America/Anchorage@AKST9AKDT,M3.2.0,M11.1.0"><?php echo language('Alaska Time');?></option>
						<option  value="America/Los_Angeles@PST8PDT,M3.2.0,M11.1.0"><?php echo language('Pacific Time');?></option>
						<option  value="America/Denver@MST7MDT,M3.2.0,M11.1.0"><?php echo language('Mountain Time');?></option>
						<option  value="America/Phoenix@MST7"><?php echo language('Mountain Time @Arizona, no DST');?></option>
						<option  value="America/Chicago@CST6CDT,M3.2.0,M11.1.0"><?php echo language('Central Time');?></option>
						<option  value="America/New_York@EST5EDT,M3.2.0,M11.1.0"><?php echo language('Eastern Time');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Atlantic');?>">
						<option  value="Atlantic/Bermuda@AST4ADT,M3.2.0,M11.1.0"><?php echo language('Bermuda');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+1)">
						<option  value="Asia/Anadyr@ANAT-12ANAST,M3.5.0,M10.5.0/3"><?php echo language('Anadyr');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+2)">
						<option  value="Asia/Amman@EET-2EEST,M3.5.4/0,M10.5.5/1"><?php echo language('Amman');?></option>
						<option  value="Asia/Beirut@EET-2EEST,M3.5.0/0,M10.5.0/0"><?php echo language('Beirut');?></option>
						<option  value="Asia/Damascus@EET-2EEST,J91/0,J274/0"><?php echo language('Damascus');?></option>
						<option  value="Asia/Gaza@EET-2EEST,J91/0,M10.3.5/0"><?php echo language('Gaza');?></option>
						<option  value="Asia/Jerusalem@GMT-2"><?php echo language('Jerusalem');?></option>
						<option  value="Asia/Nicosia@EET-2EEST,M3.5.0/3,M10.5.0/4"><?php echo language('Nicosia');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+3)">
						<option  value="Asia/Aden@AST-3"><?php echo language('Aden');?></option>
						<option  value="Asia/Baghdad@AST-3ADT,J91/3,J274/4"><?php echo language('Baghdad');?></option>
						<option  value="Asia/Bahrain@AST-3"><?php echo language('Bahrain');?></option>
						<option  value="Asia/Kuwait@AST-3"><?php echo language('Kuwait');?></option>
						<option  value="Asia/Qatar@AST-3"><?php echo language('Qatar');?></option>
						<option  value="Asia/Riyadh@AST-3"><?php echo language('Riyadh');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+3:30)">
						<option  value="Asia/Tehran@IRST-3:30"><?php echo language('Tehran');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+4)">
						<option  value="Asia/Baku@AZT-4AZST,M3.5.0/4,M10.5.0/5"><?php echo language('Baku');?></option>
						<option  value="Asia/Dubai@GST-4"><?php echo language('Dubai');?></option>
						<option  value="Asia/Muscat@GST-4"><?php echo language('Muscat');?></option>
						<option  value="Asia/Tbilisi@GET-4"><?php echo language('Tbilisi');?></option>
						<option  value="Asia/Yerevan@AMT-4AMST,M3.5.0,M10.5.0/3"><?php echo language('Yerevan');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+4:30)">
						<option  value="Asia/Kabul@AFT-4:30"><?php echo language('Kabul');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+5)">
						<option  value="Asia/Aqtobe@AQTT-5"><?php echo language('Aqtobe');?></option>
						<option  value="Asia/Ashgabat@TMT-5"><?php echo language('Ashgabat');?></option>
						<option  value="Asia/Dushanbe@TJT-5"><?php echo language('Dushanbe');?></option>
						<option  value="Asia/Karachi@PKT-5"><?php echo language('Karachi');?></option>
						<option  value="Asia/Oral@ORAT-5"><?php echo language('Oral');?></option>
						<option  value="Asia/Samarkand@UZT-5"><?php echo language('Samarkand');?></option>
						<option  value="Asia/Tashkent@UZT-5"><?php echo language('Tashkent');?></option>
						<option  value="Asia/Yekaterinburg@YEKT-5"><?php echo language('Yekaterinburg');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+5:30)">
						<option  value="Asia/Calcutta@IST-5:30"><?php echo language('Calcutta');?></option>
						<option  value="Asia/Colombo@IST-5:30"><?php echo language('Colombo');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+6)">
						<option  value="Asia/Almaty@ALMT-6"><?php echo language('Almaty');?></option>
						<option  value="Asia/Bishkek@KGT-6"><?php echo language('Bishkek');?></option>
						<option  value="Asia/Dhaka@BDT-6"><?php echo language('Dhaka');?></option>
						<option  value="Asia/Novosibirsk@NOVT-6"><?php echo language('Novosibirsk');?></option>
						<option  value="Asia/Omsk@OMST-6"><?php echo language('Omsk');?></option>
						<option  value="Asia/Qyzylorda@QYZT-6"><?php echo language('Qyzylorda');?></option>
						<option  value="Asia/Thimphu@BTT-6"><?php echo language('Thimphu');?></option>
					</optgroup>
						<optgroup label="<?php echo language('Asia');?> (UTC+7)">
						<option  value="Asia/Jakarta@WIT-7"><?php echo language('Jakarta');?></option>
						<option  value="Asia/Bangkok@ICT-7"><?php echo language('Bangkok');?></option>
						<option  value="Asia/Vientiane@ICT-7"><?php echo language('Vientiane');?></option>
						<option  value="Asia/Phnom_Penh@ICT-7"><?php echo language('Phnom Penh');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+8)">
						<option  value="Asia/Chongqing@CST-8"><?php echo language('Chongqing');?></option>
						<option  value="Asia/Hong_Kong@HKT-8"><?php echo language('Hong Kong');?></option>
						<option  value="Asia/Shanghai@CST-8"><?php echo language('Shanghai');?></option>
						<option  value="Asia/Singapore@SGT-8"><?php echo language('Singapore');?></option>
						<option  value="Asia/Urumqi@CST-8"><?php echo language('Urumqi');?></option>
						<option  value="Asia/Taipei@CST-8"><?php echo language('Taiwan');?></option>
						<option  value="Asia/Ulaanbaatar@ULAT-8ULAST,M3.5.6,M9.5.6/0"><?php echo language('Ulaanbaatar');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Asia');?> (UTC+9)">
						<option  value="Asia/Dili@TLT-9"><?php echo language('Dili');?></option>
						<option  value="Asia/Jayapura@EIT-9"><?php echo language('Jayapura');?></option>
						<option  value="Asia/Pyongyang@KST-9"><?php echo language('Pyongyang');?></option>
						<option  value="Asia/Seoul@KST-9"><?php echo language('Seoul');?></option>
						<option  value="Asia/Tokyo@JST-9"><?php echo language('Tokyo');?></option>
						<option  value="Asia/Yakutsk@YAKT-9YAKST,M3.5.0,M10.5.0/3"><?php echo language('Yakutsk');?></option>
					</optgroup>
					<optgroup label="<?php echo language('Central and South America');?>">
						<option  value="America/Sao_Paulo@BRT3BRST,M11.1.0/0,M2.5.0/0"><?php echo language('Sao Paulo,Brazil');?></option>
						<option  value="America/Buenos_Aires@ART3"><?php echo language('Buenos_Aires,Argentina');?></option>
						<option  value="America/Guatemala@CST6"><?php echo language('Central America @no DST');?></option>
						<option  value="America/Caracas@VET4:30"><?php echo language('Caracas,Venezuela');?></option>
					</optgroup>
					</select>
			</div>
		</div>
	</div>
	<div class="ant-row ant-form-item">
		<div class="ant-col-4 ant-form-item-label">
			<label for="self_defined_time_zone" class="" title=""><span><?php echo language("Self-defined Time Zone");?></span></label>
		</div>
		<div class="ant-col-6 ant-form-item-control-wrapper">
			<div class="ant-form-item-control has-success">
				<input type="text" value="" id="self_defined_time_zone" name="self_defined_time_zone" data-__meta="[object Object]" class="ant-input ant-input-lg">
			</div>
		</div>
	</div>
	<div class="ant-row ant-form-item">
		<div class="ant-col-4 ant-form-item-label">
			<label for="lan2_ip_method" class="" title=""><span><?php echo language("Language");?></span></label>
		</div>
		<div class="ant-col-6 ant-form-item-control-wrapper">
			<div class="ant-form-item-control has-success">
				<select id="language_type" name="language_type" class="ant-select-selection-selected-value">
					<option value="chinese" ><?php echo '中文';?></option>
					<option value="english" ><?php echo language('english');?></option>
				</select>
			</div>
		</div>
	</div>
</div>
