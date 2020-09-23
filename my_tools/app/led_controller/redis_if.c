#include <stdio.h>
#include <pthread.h>
#include "hiredis.h"

#define DEFAULT_REDIS_IP "127.0.0.1"
#define DEFAULT_REDIS_PORT 6379
#define MAX_TAG_LEN 32
#define MAX_VALUE_LEN 32
#define _SIMSLOT_LIST_ "app.ledserver.simslot"
#define _BOARD_SPAN_   "local.product.board.span"
#define _BOARD_TYPE_   "local.product.board.type"
#define _redis_info_tag_sim_ "sim"
#define _redis_info_tag_slot_ "slot"
#define AST_SPAN_MAX 32

typedef struct simslot_info_t{
	int slot;
}SIMSLOT_INFO_T;

typedef enum simslot_id_e{
	SIMSLOT_ID_SIM = 0,
	SIMSLOT_ID_SLOT,
	SIMSLOT_ID_MAX
}SIMSLOT_ID_E;

typedef struct _simslot_tag_value_s{
	const char *tag;
	char value[MAX_VALUE_LEN];
}SIMSLOT_TAG_VALUE_S;


SIMSLOT_TAG_VALUE_S g_tag_value_info[] = {
    {_redis_info_tag_sim_,      ""},
    {_redis_info_tag_slot_,       ""},
};

SIMSLOT_INFO_T simslot[AST_SPAN_MAX];

redisContext *redis_cli;

static redisContext *conn_redis()
{
    redisContext *c = NULL;

    c = redisConnect(DEFAULT_REDIS_IP, DEFAULT_REDIS_PORT);
    if (c == NULL || c->err) {
        printf("goto redisConnect failed.\n");
        return NULL;
    }   

    return c;

}
static char *trim_quote(char *src, char *dst)
{   
    int len;
    int tail; 
    int head = 0; 
        
    if (src == NULL || dst == NULL)
        return NULL;

    len = strlen(src);
    tail = len;
    if (src[0] == '"')
        head = 1;
    if (src[len-1] == '"')
        tail -= 1;
    len = tail - head;
    strncpy(dst, &src[head], len);
    dst[len] = '\0';

    return dst;
}

static char *get_redis_tv(const char *buf, char token, char *tag, char *value)      // token is =
{
    int opt = 0;
    char *find = NULL;
    char *left = NULL;
    char *right = NULL;
    char *ptr = (char *)buf;
    char key_str[4] = {'"',token,'"',0};

    find = strstr(ptr, key_str);
    if (find) {
        opt = find - ptr;
        right = find + 2;
        *(find+1) = '\0';
        left = ptr;
        if (tag && left && opt < MAX_TAG_LEN) {
            trim_quote(left, tag);
        }
        if (value && right && strlen(right) < MAX_VALUE_LEN) {
            trim_quote(right, value);
        }
    }

    return right;
}


char* get_redis_info_token(const char *start, char info_token, char tv_token, char *tag, char *value)
{
    int  opt = 0;
    char *find = NULL;
    char *left = NULL;
    char *right = NULL;
    char *ptr = (char *)start;
    char key_str[4] = {'"',info_token,'"',0};
    
    if(start == NULL)
        return NULL;
    opt = strlen(start);
    if(start[opt-1] == '}')
        ptr[opt-1] = '\0';
    if(start[0] == '{')
        ptr += 1;

    find = strstr(ptr,key_str);
    if(find) {
        opt = find - ptr;
        right = find + 2;
        *(find+1) = '\0';
        left = ptr;
        get_redis_tv((const char *)left, tv_token, tag, value);
    } else {
        //This is last info
        get_redis_tv((const char *)ptr, tv_token, tag, value);
    }

    return right;
}


int get_redis_sim_slot_info(redisContext *redis_c, int *sim_num, int *slot_num)
{
	redisReply *reply = NULL;
	char *ptr = NULL;
	char *command = _SIMSLOT_LIST_;
	char left[MAX_TAG_LEN];
	char right[MAX_VALUE_LEN];
	int i = 0, ret = -1;
	
	reply = redisCommand(redis_c, "blpop %s 0", _SIMSLOT_LIST_);
	if(reply == NULL){
		printf("get sim slot info from redis faile, reply is NULL\n");
		return ret;
	}

	if (REDIS_REPLY_ARRAY == reply->type && 2 == reply->elements ) {
		if (reply->element[1] != NULL && reply->element[1]->str != NULL) {
			ptr = reply->element[1]->str;
			while (ptr) {
				left[0] = '\0';
				right[0] = '\0';
				ptr = get_redis_info_token((const char *)ptr, ',', ':', left, right);
				if (strlen(left) == 0)
					break;
				for (i = 0; i < SIMSLOT_ID_MAX; i++) {
					if (strcmp(left, g_tag_value_info[i].tag) == 0) {
						if(i == SIMSLOT_ID_SIM)
							*sim_num = atoi(right);
						else if(i == SIMSLOT_ID_SLOT)
							*slot_num = atoi(right);
					}
				}
		//		printf("sim_num = %d, slot_num = %d\n");
				if(*sim_num > 0 && *sim_num <= AST_SPAN_MAX && *slot_num > 0 && *slot_num <= 4)
					ret = 0;
				else
					ret = -1;
			}
		}
	}
	return ret;
}

static void *led_redis_pthread_cb_handle(int sys_type)
{
	int ret;
	int sim_num;
	int slot_num;
	memset(simslot, 0, sizeof(simslot));
	//Wait until redis is up and running
	while((redis_cli = conn_redis()) == NULL ) sleep(1);

	while(redis_cli != NULL){
		
		ret = get_redis_sim_slot_info(redis_cli,&sim_num, &slot_num);

		if(!ret){
			simslot[sim_num - 1].slot = slot_num;
		}

		usleep(20*1000);
	}

	if(redis_cli != NULL)
		redisFree(redis_cli);
	redis_cli = NULL;
}

void led_create_redis_thread(int sys_type)
{
	int ret;
	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	 
	if(sys_type != 2){//not 1chan 4 sim
		memset(simslot, 0, sizeof(simslot));
		return;
	}

	ret = pthread_attr_setstacksize(&attr, 1024 * 1024);
	if(0 != ret){
		printf("pthread_attr_setstacksize failed.\n");
		return;
	} 
		
	ret = pthread_create(&tid, &attr, led_redis_pthread_cb_handle, simslot);
	if(ret < 0){
		printf("create led redis thread fail\n");
	}
	
}

int get_sim_slot_info(char *out){
	int i = 0;
	//not 1chan 4sim device
	for(i = 0; i < AST_SPAN_MAX; i++){
		if(simslot[i].slot == 0){
			sprintf(&out[i], "%c", ' ');	
		}else{
			sprintf(&out[i], "%c", simslot[i].slot + '0');	
		}
	}
	return 0;
}


int get_total_channel()
{
	int total_channel = 0;
	redisContext *redis_c = NULL;
	redisReply *reply = NULL;
	char *command = _BOARD_SPAN_;

	redis_c = conn_redis();
	if (redis_c == NULL)
		return 0;

	reply = redisCommand(redis_c, "GET %s", command);
	if (NULL == reply) {
		printf("Goto redisCommand failed. reply is NULL\n");
		redisFree(redis_c);
		return 0;
	}
	if (reply->type != REDIS_REPLY_ERROR && reply->str != NULL) {
		total_channel = atoi(reply->str);
	}
	freeReplyObject((void *)reply);
	redisFree(redis_c);

	return total_channel;
}

int get_sys_type()
{
	int sys_type = 0;
	redisContext *redis_c = NULL;
	redisReply *reply = NULL;
	char *command = _BOARD_TYPE_;

	redis_c = conn_redis();
	if (redis_c == NULL)
		return 0;

	reply = redisCommand(redis_c, "GET %s", command);
	if (NULL == reply) {
		printf("Goto redisCommand failed. reply is NULL\n");
		redisFree(redis_c);
		return 0;
	}
	if (reply->type != REDIS_REPLY_ERROR && reply->str != NULL) {
		sys_type = atoi(reply->str);
	}
	freeReplyObject((void *)reply);
	redisFree(redis_c);

	return sys_type;
}
