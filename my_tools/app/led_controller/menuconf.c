#include <stdio.h>
#include <string.h>
#include "menuctrl.h"


static int get_option_value(const char * file_path, const char * context_name, const char * option_name, char * out_value)
{
	char buf[LINE_SIZE];
	int s;
	int len;
	int out;
	int i;
	int finded = 0;
	int finish = 0;
	FILE* fp;
	char name[NAME_SIZE];


	if( NULL == (fp=fopen(file_path,"r")) ) {
		mconf_print_error("%s Can't open %s\n", file_path, __FUNCTION__);
		return -1;
	}

	while(fgets(buf,LINE_SIZE,fp)) {
		s = -1;
		out = 0;
		len = strlen(buf);
		for( i=0; i<len; i++ ) {
			switch( buf[i] ) {
			case '#':
			case '\r':
			case '\n':
				out=1;
				break;
			case '[':
				s=i;
				break;
			case ']':
				if( s != -1 ) {
					memcpy(name,buf+s+1,i-s-1);
					name[i-s-1] = '\0';

					if( 0== strcmp(name,context_name) ) {
						finded=1;
					} else {
						if(finded) 
							finish=1;
					}
				}
				break;
			case '=':
				if(finded && !finish) {
					memcpy(name,buf,i);
					name[i] = '\0';
					if(0==strcmp(name,option_name)) {
						memcpy(name,buf+i+1,len-i-1-1);
						name[len-i-1-1] = '\0';
						fclose(fp);
						memcpy(out_value,name,len-i-1);
						return 0;
					}
				}
			}
			if(out)
				break;
		}
	}

	fclose(fp);
	return -1;
}


static int menu_set_default_conf(menuconf_t *pconf)
{
	if(!pconf) {
		mconf_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}

	pconf->NokeyTime = NO_KEY_CNT;
	pconf->PowerSaveFlag = 0;
	pconf->PowerSaveTime = DORM_CNT;
	return 0;
}



int menu_read_conf(menuconf_t *pconf)
{
	char tmp[NAME_SIZE] = {0};
	int res = -1;

	if(!pconf) {
		mconf_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}	

	if(!(res = get_option_value(MENUCONFPATH, CONTEXT_MENUCONF, OPTION_POWERSAVEFLAG, tmp))) {
		pconf->PowerSaveFlag = atoi(tmp);
//		mconf_print_debug("%s pconf->PowerSaveFlag  = %d.\n", __FUNCTION__, pconf->PowerSaveFlag);
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(MENUCONFPATH, CONTEXT_MENUCONF, OPTION_POWERSAVETIME, tmp))) {
		pconf->PowerSaveTime = atoi(tmp)*1000000/SCAN_CYCLE;
//		mconf_print_debug("%s pconf->PowerSaveTime = %d.\n", __FUNCTION__, pconf->PowerSaveTime);
	}
	else {
		goto _error;
	}

	if(!(res = get_option_value(MENUCONFPATH, CONTEXT_MENUCONF, OPTION_NOKEYTIME, tmp))) {
		pconf->NokeyTime = atoi(tmp)*1000000/SCAN_CYCLE;
//		mconf_print_debug("%s pconf->NokeyTime = %d.\n", __FUNCTION__, pconf->NokeyTime);
	}
	else {
		goto _error;
	}

	return 0;
_error:
	menu_set_default_conf(pconf);
	return -1;	
}


int menu_reflesh_conf(menuconf_t *pconf)
{
	FILE *fp;
	int i = 0;
	char buff[LINE_SIZE] = {0};
	char dev_context[20] = {0}; 

	if(!pconf) {
		mconf_print_error("%s input fail.\n", __FUNCTION__);
		return -1;
	}	

	if( NULL == (fp = fopen(MENUCONFPATH, "r+")) ) {
		mconf_print_error("%s Can't open %s\n", __FUNCTION__, MENUCONFPATH);
		return -1;
	}

	fprintf(fp, "[menu]\n");
	fprintf(fp, "powersaveflag=%d\n", pconf->PowerSaveFlag);
	fprintf(fp, "powersavetime=%d\n", pconf->PowerSaveTime*SCAN_CYCLE/1000000);
	fprintf(fp, "nokeytime=%d\n", pconf->NokeyTime*SCAN_CYCLE/1000000);
	fclose(fp);
	return 0;
}


