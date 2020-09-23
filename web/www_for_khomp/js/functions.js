function value(name)
{
	var item = document.getElementById(name);
	return (item ? item.value : "");
}
function set_value(name, value)
{
	var item = document.getElementById(name);
	if (item) item.value = value;
}
function isset(name, val)
{
	return (value(name) == val);
}
function checked(name)
{
	var item = document.getElementById(name);
	return ((item) && item.checked);
}
function hide(name)
{
	var item = document.getElementById(name);
	if (item) 
		item.style.display = 'none';
}
function show(name)
{
	var item = document.getElementById(name);
	if (item)
		item.style.display = '';
}
function set_visible(name, value)
{
	if (value)
		show(name)
	else
		hide(name)
}

function trim(s)
{
	//return s.replace( /^\s*/, "" ).replace( /\s*$/, "" );
	return s.replace(/(^\s*)|(\s*$)/g, ""); 
}

//IP转成无符号数值  
function ip2num(ip)   
{  
    var num = 0;  
    ip = ip.split(".");  
    num = Number(ip[0]) * 256 * 256 * 256 + Number(ip[1]) * 256 * 256 + Number(ip[2]) * 256 + Number(ip[3]);  
    num = num >>> 0;  
    return num;  
}  
  
//无符号转成IP地址  
function num2ip(num)   
{  
    var str;  
    var tt = new Array();  
    tt[0] = (num >>> 24) >>> 0;  
    tt[1] = ((num << 8) >>> 24) >>> 0;  
    tt[2] = (num << 16) >>> 24;  
    tt[3] = (num << 24) >>> 24;  
    str = String(tt[0]) + "." + String(tt[1]) + "." + String(tt[2]) + "." + String(tt[3]);  
    return str;  
}  



// 说明：Javascript 控制 CheckBox 的全选与取消全选
// 整理：http://www.CodeBit.cn
function checkAll(name)
{
	var el = document.getElementsByTagName('input');
	var len = el.length;
	for(var i=0; i<len; i++)
	{
	    if((el[i].type=="checkbox") && (el[i].name==name))
	    {
		el[i].checked = true;
	    }
	}
}

function clearAll(name)
{
	var el = document.getElementsByTagName('input');
	var len = el.length;
	for(var i=0; i<len; i++)
	{
	    if((el[i].type=="checkbox") && (el[i].name==name))
	    {
		el[i].checked = false;
	    }
	}
}

function checkSelect(name)
{
	//判断checkbox是否被选中
	var el = document.getElementsByTagName('input');
	var len = el.length;
	for (i = 0; i < len; i++) 
	{
		if((el[i].type=="checkbox") && (el[i].name==name))
		{
			if (el[i].checked == true) 
			{
				return true;
			}
		} 
	} 
	return false;
}

function selectAll(checked,name)
{
	if(checked == true) {
		checkAll(name);
	} else {
		clearAll(name);
	}
}

function selectAllCheckbox(checked,attr,value)
{
	var inputs = document.getElementsByTagName("input");
	var len = inputs.length;
	for(var i=0; i<len; i++){
		//if(inputs[i].getAttribute("type")=="checkbox" && inputs[i].getAttribute(attr)==value){
		if(inputs[i].getAttribute("type")=="checkbox" && inputs[i].className==value){
			inputs[i].checked = checked;
		}
	}
}

function isCheckboxChecked(attr,value)
{
	var inputs = document.getElementsByTagName("input");
	var len = inputs.length;
	for(var i=0; i<len; i++){
		//if(inputs[i].getAttribute("type")=="checkbox" && inputs[i].getAttribute(attr)==value){
		if(inputs[i].getAttribute("type")=="checkbox" && inputs[i].className==value){
			if(inputs[i].checked==true){
				return true;
			}
		}
	}
	return false;
}

function isAllCheckboxChecked(attr,value)
{
	var inputs = document.getElementsByTagName("input");
	var len = inputs.length;
	for(var i=0; i<len; i++){
		//if(inputs[i].getAttribute("type")=="checkbox" && inputs[i].getAttribute(attr)==value){
		if(inputs[i].getAttribute("type")=="checkbox" && inputs[i].className==value){
			if(inputs[i].checked!=true){
				return false;
			}
		}
	}
	return true;
}

function setCookie(name,value)
{
	var Days = 10;
	var exp  = new Date();    
	exp.setTime(exp.getTime() + Days*24*60*60*1000);
	document.cookie = name + "="+ escape (value) + ";expires=" + exp.toGMTString();
}

function getCookie(name) 
{
	var arr = document.cookie.match(new RegExp("(^| )"+name+"=([^;]*)(;|$)"));
	if(arr != null) return unescape(arr[2]); return null;
}

function delCookie(name)
{
	var exp = new Date();
	exp.setTime(exp.getTime() - 1);
	var cval=getCookie(name);
	if(cval!=null) document.cookie= name + "="+cval+";expires="+exp.toGMTString();
}

function createHideValue(name, value)
{
	var input = document.createElement("input");
	input.type = 'hidden';
	input.value = value;
	o.appendChild(input);
}


/*Switch download from internet by Freedom*/
/*Like iphone switch*/
$(document).ready( function(){ 
	$(".cb-enable").click(function(){
		var parent = $(this).parents('.switch');
		$('.cb-disable',parent).removeClass('selected');
		$(this).addClass('selected');
		$('.checkbox',parent).attr('checked', true);
	});
	$(".cb-disable").click(function(){
		var parent = $(this).parents('.switch');
		$('.cb-enable',parent).removeClass('selected');
		$(this).addClass('selected');
		$('.checkbox',parent).attr('checked', false);
	});
});
