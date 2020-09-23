
/*function lud(oSourceObj,oTargetObj,shutAble,oOpenTip,oShutTip)
{
	var sourceObj = typeof oSourceObj == "string" ? document.getElementById(oSourceObj) : oSourceObj;
	var targetObj = typeof oTargetObj == "string" ? document.getElementById(oTargetObj) : oTargetObj;
	var openTip = oOpenTip || "";
	var shutTip = oShutTip || "";

	if(targetObj.style.display!="none"){
		if(shutAble) return;
		targetObj.style.display="none";
		document.getElementById(oTargetObj+"_li").className="tb5";
		if(openTip && shutTip) {
			sourceObj.innerHTML = shutTip;
		}
	} else {
		targetObj.style.display="block";
		document.getElementById(oTargetObj+"_li").className="tb4";
		if(openTip && shutTip) {
			sourceObj.innerHTML = openTip;
	   }
	}
}*/

function lud(oSourceObj,oTargetObj,shutAble,oOpenTip,oShutTip)
{
	var sourceObj = typeof oSourceObj == "string" ? document.getElementById(oSourceObj) : oSourceObj;
	var targetObj = typeof oTargetObj == "string" ? document.getElementById(oTargetObj) : oTargetObj;
	var openTip = oOpenTip || "";
	var shutTip = oShutTip || "";

	if(targetObj.style.display!="none"){
		if(shutAble) return;
		targetObj.style.display="none";
		document.getElementById(oTargetObj+"_li").className="tb_fold";
		if(openTip && shutTip) {
			sourceObj.innerHTML = shutTip;
		}
	} else {
		targetObj.style.display="block";
		document.getElementById(oTargetObj+"_li").className="tb_unfold";
		if(openTip && shutTip) {
			sourceObj.innerHTML = openTip;
	   }
	}
}

function switch_over(num,nav_lists_count){
	for(var id = 0; id < nav_lists_count ; id++)
	{
		if(id==num)
		{
			document.getElementById("qh_con"+id).style.display="block";
			document.getElementById("mynav"+id).className="nav_on";
		} else {
			document.getElementById("qh_con"+id).style.display="none";
			document.getElementById("mynav"+id).className="";
		}
	}
}