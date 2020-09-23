 /**
 * 模拟linux终端
 *
 * 功能:
 * 1.支持history
 * 1.history 显示所有执行成功过的命令 (编号命令)
 * 2.history -i 执行第i个命令
 * 3.up 上一个命令
 * 4.down 下一个命令
 * 2.支持backspace,左方向键,右方向键
 * 1.backspace 删一个字
 * 2.左方向键 光标向左移动一个字
 * 3.右方向键 光标向右移动一个字
 * 3.组合键
 * 1.ctrl+a home键 光标移到行首
 * 2.ctrl+e end 键 光标移到行未

 * 4.ctrl+d(delete) 删除当前行
 * 4.enter 命令输入完毕
 * 5.退出 esc ctrl+c
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <termios.h>

#include "cli.h"

#define MAX_LINE_LENGTH 1024 //每行最大字符数
#define MAX_PROMPT_LENGTH 64 //提示符最大的长度
#define MAX_HISTORY_FILE_PATH_LENGTH 64 //保存路径的最大长度
#define HISTORY_FILE_PATH "/tmp/cli_history" //历史命令保持的路径
#define DEFAULT_PROMPT "bsp>" //默认提示符

#define CLI_ERR 0
#define CLI_OK 1

typedef void (*pFUNC_table_handle)(char *line_text);    /* 按下table键的回调函数 */

/* 回调函数节点结构体，二维树 */
struct NODE_CALLBACK{
    char *name;                     /* 当前节点名称 */
    pFUNC_CALLBACK cb;              /* 回调函数 */
    struct NODE_CALLBACK *bro;      /* brother(兄弟)节点 */
    struct NODE_CALLBACK *son;      /* 子节点 */
};

/*API*/
void loadHistoryCmd(char *history_file_path);
int  appendCommandToHistory(char *cmd);
char *cli_run(const char*prompt);

/*把终端的一行抽象为line*/
struct cli_line{
 //保存的字符数
 char buf[MAX_LINE_LENGTH];
 //写指针
 int pos;
 //字符长度;
 int buf_length;
 //光标的位置
 int cursor_x;
 //提示字符串 长度为63个字节
 char prompt[MAX_PROMPT_LENGTH];
 /*提示 字符串的长度*/
 int prompt_length;
 /*history file path*/
 char history_file_path[MAX_HISTORY_FILE_PATH_LENGTH];
 int ifd;
 int ofd;
};

/*history command structrue*/
#define MAX_HISTORY_COUNT 100
struct historyCmdTable{
 char *cmd[MAX_HISTORY_COUNT];
 int count;
};

struct historyCmdTable history;
struct cli_line l;
static int history_count =0;
static int key_current_index =0;

/* 回调函数节点全局变量，头节点 */
struct NODE_CALLBACK *g_cb_head;

struct time{
    int tm_sec; //代表目前秒数, 正常范围为0-59, 但允许至61 秒
    int tm_min; //代表目前分数, 范围0-59
    int tm_hour; //从午夜算起的时数, 范围为0-23
    int tm_mday; //目前月份的日数, 范围01-31
    int tm_mon; //代表目前月份, 从一月算起, 范围从0-11
    int tm_year; //从1900 年算起至今的年数
    int tm_wday; //一星期的日数, 从星期一算起, 范围为0-6
    int tm_yday; //从今年1 月1 日算起至今的天数, 范围为0-365
    int tm_isdst; //日光节约时间的旗标
};
char *get_current_time(){
    char *wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    time_t timep;
    struct time *p;
    time(&timep);
    p = (struct time *)gmtime(&timep);
    char *buf=(char*)malloc(100);
    if(buf==NULL) return NULL;
	memset(buf, 0, 100);
    snprintf(buf,100,"%d/%d/%d ", (1900+p->tm_year), (1+p->tm_mon), p->tm_mday);
    snprintf(buf+strlen(buf),100-strlen(buf),"%s %d:%d:%d", wday[p->tm_wday], p->tm_hour, p->tm_min, p->tm_sec);
    return buf;
}

#define LOG_LEVEL_WARNING 1 /*只是提示信息，不终止程序*/
#define LOG_LEVEL_ERROR 2 /*会终止程序*/
#define LOG_FILE_PATH "./log.txt"
#define LOG_OPEN_FAILURE "unable to open the log file!"
static void logs(int level,char *message,int is_show){
     /*is_show 内容是否需要立即显示*/
     if(is_show || level==LOG_LEVEL_ERROR){
      fprintf(stdout,message);
      fprintf(stdout,"\n");
     }
     FILE *fp = fopen(LOG_FILE_PATH,"a+");
     if(fp!=NULL){
      char buf[1024] = {0};
      if(level==LOG_LEVEL_WARNING)
       snprintf(buf,sizeof(buf),"[%s][warning] %s\n",get_current_time(),message);
      if(level==LOG_LEVEL_ERROR)
       snprintf(buf,sizeof(buf),"[%s][error] %s\n",get_current_time(),message);
      fwrite(buf,strlen(buf),1,fp);
      fclose(fp);
      if(level==LOG_LEVEL_ERROR) exit(1);
     }else{
      printf("\nfile:%s,line:%d, warning:%s\n",__FILE__,__LINE__,strerror(errno));
     }
}

static void vsnlogs(int level,const char*fmt,...){
     char buf[128] = {0};
     va_list ap;
     va_start(ap,fmt);
     vsnprintf(buf,sizeof(buf),fmt,ap);
     va_end(ap);
     logs(level,buf,0);
}

/*append cmd to history*/
int appendCommandToHistory(char *cmd){
     struct historyCmdTable *history1 = &history;
     if(!cmd){
      return CLI_ERR;
     }
     int cmdLength = strlen(cmd);
     char *p = (char*)malloc(cmdLength+1);
     if(p==NULL){
        return CLI_ERR;
     }
	 memset(p, 0, cmdLength + 1);
     memcpy(p,cmd,cmdLength);
     p[cmdLength] = '\0';
     if(history_count-1 >=0 && strcmp(history1->cmd[history_count-1],cmd)==0){
        key_current_index = history_count;
        return CLI_OK;
     }
     history1->cmd[history_count ++] = p;
     if(history_count>MAX_HISTORY_COUNT){
     	    //free(history->cmd[0]);
          memmove(history1->cmd,history1->cmd+1,sizeof(char*)*(history_count-1));
          history_count--;
     }
     //printf("\nhistory_count=%d,MAX_HISTORY_COUNT=%d\n",history_count,MAX_HISTORY_COUNT);
     key_current_index = history_count ;
     vsnlogs(LOG_LEVEL_WARNING,"add history:%s,id:%d",cmd,history_count-1);
     return CLI_OK;
}

/*load history cmd from history_file_path*/
void loadHistoryCmd(char *history_file_path){
      strcpy(l.history_file_path,history_file_path);
     //struct historyCmdTable *history = &history;
     FILE *fp = fopen(history_file_path,"r");
     if(fp==NULL){
          return ;
     }
     char buf[MAX_LINE_LENGTH] = {0};
     char *p;
     while(fgets(buf,MAX_LINE_LENGTH,fp)!=NULL){
        p = strchr(buf,'\r');
        if(!p) p = strchr(buf,'\n');
        if(p) *p= '\0';
        appendCommandToHistory(buf);
     }
}

/*fflush the history cmd to disk*/
static void saveHistoryCmd( struct historyCmdTable *history){
     char *history_file_path = l.history_file_path;
     FILE *fp = fopen(history_file_path,"w+");
     char buf[1024] = {0};
     if(fp){
      int i=0;
      for(i=0;i<history_count;i++){
       snprintf(buf,sizeof(buf),"%s\n",history->cmd[i]);
       //printf("%d,%s\n",i+1,history->cmd[i]);
       fwrite(buf,strlen(buf),1,fp);
      }
      fclose(fp);
     }else{
        printf("\nsave history command error!\n");
     }
}

/*init var line ,hisotry*/
static void init(){
     strcpy(l.prompt,DEFAULT_PROMPT);
     l.prompt_length = strlen(DEFAULT_PROMPT);
     l.buf[0] = '\0';
     l.cursor_x = 0;
     l.pos = 0;
     l.buf_length = 0;
     l.ifd = STDIN_FILENO;
     l.ofd = STDOUT_FILENO;
}

static char *print_r(struct historyCmdTable *history,int index){
     int i= 0;
     if(history_count==0){
        vsnlogs(LOG_LEVEL_WARNING,"\nwarning: no history command loaded!\n");
        printf("\nempty set\n");
        init();
        return NULL;
     }
     if(index!=-1){
        if(index>=1 && index<=history_count){
         //printf("\n%d\t%s\n",index,history->cmd[index-1]);
         return strdup(history->cmd[index-1]);
        }else{
         char buf[1024];
         snprintf(buf,sizeof(buf),"\nfile:%s line:%d %s(index=%d,history_count=%d)",
          __FILE__,__LINE__,"unexcpted index!",index,history_count);
         logs(LOG_LEVEL_WARNING,buf,1);
        }
     }else{
        printf("\n");
        for(i=0;i<history_count;i++){
			printf("%d\t%s\n",i+1,history->cmd[i]);
        }
     }
     return NULL;
}
struct lineStream{
 char buf[1024];
 int length;
};

/*缓冲到lineStream,一起输出，防止flcker*/
static void appendLineStream(struct lineStream *ls,const char*fmt,...){
     char buf[128] = {0};
     int length;
     va_list ap;
     va_start(ap,fmt);
     vsnprintf(buf,sizeof(buf),fmt,ap);
     va_end(ap);
     length = strlen(buf);
     if(length+ls->length>1024){
      snprintf(buf,sizeof(buf),"%s,%d,length(%d)+ls->length(%d)>1024",__FILE__,__LINE__,length,ls->length);
      logs(LOG_LEVEL_WARNING,buf,1);
     }else{
      memcpy(ls->buf+ls->length,buf,length);
      ls->length +=length;
      ls->buf[ls->length] = '\0';
     }
}

static void refreshLine(struct cli_line *l){
  struct lineStream *ls = (struct lineStream*)malloc(sizeof(struct lineStream));
  memset(ls, 0 , sizeof(struct lineStream));
  /*移动光标到行首*/
  appendLineStream(ls,"\x1b[0G");
  appendLineStream(ls,"\x1b[0K");

  appendLineStream(ls,l->prompt);
  appendLineStream(ls,l->buf);

  /*移动光标到cursor_x*/
  appendLineStream(ls,"\x1b[0G\x1b[%dC",l->pos + l->prompt_length);
  if(write(STDOUT_FILENO,ls->buf,ls->length)<0){
   char message[128];
   snprintf(message,sizeof(message),"file=%s,line=%d,wirte(STDOUT_FILENO,%s,%d)<0",
    __FILE__,__LINE__,ls->buf,ls->length);
   logs(LOG_LEVEL_WARNING,message,1);
  }
}

/*改变prompt*/
static void setPrompt(const char*fmt,...){
     char buf[MAX_PROMPT_LENGTH] = {0};
     va_list ap;
     va_start(ap,fmt);
     vsnprintf(buf,sizeof(buf),fmt,ap);
     va_end(ap);
     int promptLen = strlen(buf);
     if(promptLen<=MAX_PROMPT_LENGTH){
       strcpy(l.prompt,buf);
       l.prompt_length = promptLen;
     }
}
/*向l->buf插入一个字节*/
static void lineInsertChar(struct cli_line *l,char c){
    /*中间插入*/
    if(l->pos < l->buf_length){
         memmove(l->buf+l->pos+1,l->buf+l->pos,l->buf_length - l->pos +1);
         l->buf[l->pos++] = c;
         l->buf_length ++;
         refreshLine(l);
    /*末尾插入*/
    }else{
         l->buf[l->pos] = c;
         l->pos ++;
         l->buf_length ++;
         l->buf[l->pos] = '\0';
         if(write(l->ifd,&c,1)<0){
            vsnlogs(LOG_LEVEL_WARNING,"file=%s,line=%d,errstr=%s",__FILE__,__LINE__,strerror(errno));
         }
    }
}

/*删除一个字符*/
static void lineDeleteChar(struct cli_line *l){
     if(l->pos>0){
      if(l->pos<l->buf_length){
       memmove(l->buf + l->pos-1,l->buf+l->pos,l->buf_length-l->pos+1);
       l->pos--;
       l->buf_length --;
      }else{
       l->buf[l->pos-1] = '\0';
       l->pos --;
       l->buf_length --;
      }
     }
     refreshLine(l);
}

/*光标移动到行首*/
static void cursorMoveToHome(struct cli_line *l){
   l->pos= 0;
   refreshLine(l);
}

/*光标移动到行未*/
static void cursorMoveToEnd(struct cli_line *l){
   l->pos = l->buf_length;
   refreshLine(l);
}

/*向左移动一个字符*/
void cursorMoveLeft(struct cli_line *l){
   if(l->pos<=0){
    l->pos = 0;
   }else
    l->pos--;
   refreshLine(l);
}

/*光标移动到中间*/
void cursorMoveCenter(struct cli_line *l){
    l->pos = l->buf_length/2;
    refreshLine(l);
}

/*向右移动一个字符*/
static void cursorMoveRight(struct cli_line *l){
     l->pos++;
     if(l->pos>=l->buf_length){
      l->pos = l->buf_length;
     }
     refreshLine(l);
}
static void deleteLine(struct cli_line *l){
     l->buf[0] = '\0';
     l->pos = 0;
     l->buf_length = 0;
     refreshLine(l);
}

static void keyUpHandle(struct cli_line *l){
     key_current_index --;
     vsnlogs(LOG_LEVEL_WARNING,"keyUpHandle(),key_current_index=%d",key_current_index);
     if(key_current_index<0){
        if(key_current_index <-1){
            key_current_index ++;
        }
        deleteLine(l);
     }else{
        strcpy(l->buf,history.cmd[key_current_index]);
        l->buf_length = strlen(l->buf);
        l->pos = l->buf_length;
        refreshLine(l);
     }
}

static void keyDownHandle(struct cli_line *l){
     key_current_index ++;
     vsnlogs(LOG_LEVEL_WARNING,"keyDownHandle(),key_current_index=%d",key_current_index);
     if(key_current_index >=history_count){
        if(key_current_index>=history_count+1)
          key_current_index--;
        deleteLine(l);
     }else{
        strcpy(l->buf,history.cmd[key_current_index]);
        l->buf_length = strlen(l->buf);
        l->pos = l->buf_length;
        refreshLine(l);
     }
}

/*----------------------定义键盘事件------------------------------*/

#define KEY_UP 0xE048
#define KEY_DOWN 0xE050
#define KEY_RIGHT 0xE04D
#define KEY_LEFT 0xE04B
#define KEY_TAB 0x0009
#define KEY_ENTER 0x000A
#define KEY_ESC 0x001B
#define KEY_SPACE 0x0020
#define KEY_BACKSPACE 0x0008
#define KEY_CTRL_A 0x0001 //回到行首
#define KEY_CTRL_D 0x0004 //删除一行
#define KEY_CTRL_E 0x0005 //回到行未
#define KEY_CTRL_C 0x0003 //定位到行中间

/* table键回调函数 */
pFUNC_table_handle tab_handle = NULL;

void tab_handle_set(pFUNC_table_handle pfunc)
{
    tab_handle = pfunc;
}

int getattr(struct termios *oldt)
{
    if ( -1 == tcgetattr(STDIN_FILENO, oldt) )
    {
        perror("Cannot get standard input description");
        return 1;
    }
    return 0;
}
struct termios newt,oldt;
int set_termi_attr(){
 
 getattr(&oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO | ISIG);
    //标准输入默认为行输入,去掉回显,去掉信号的通知
    newt.c_cc[VTIME] = 0;
    newt.c_cc[VMIN] = 1;
    //输入一个字符后立即返回
    //将新的属性返回给终端，并立即生效
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    return 0;
}

char **explode(char *str, char *sep,int *count){
     int sepLength = strlen(sep);
     int pos =0;
     int start_index =0;
     int end_index =0;
     int len =0;
     char **arr=NULL,*temp=NULL;
     int slots=5;
     int elements=0;
     arr = (char**)malloc(sizeof(char*)*slots);
     if(arr==NULL) goto cleanup;
     while(1){
        if(str[pos]=='\0'){
            end_index = pos-1;
            len = end_index-start_index+1;
            if(len>0){
             temp = (char*)malloc(len+1);
			 memset(temp, 0, len+1);
             if(temp==NULL) goto cleanup;
              memcpy(temp,str+start_index,len);
              temp[len] = '\0';
              elements ++;
              if(elements>slots){
                 arr = (char**)realloc(arr,sizeof(char*)*elements);
                 if(temp==NULL) goto cleanup;
              }
              arr[elements-1] = temp;
            }
            break;
        }
        if(str[pos]==sep[0] && memcmp(str+pos,sep,sepLength)==0){
            end_index = pos-1;
            len = end_index-start_index+1;
            if(len>0){
              temp = (char*)malloc(len+1);
			  memset(temp, 0, len+1);
              if(temp==NULL) goto cleanup;
              memcpy(temp,str+start_index,len);
              temp[len] = '\0';
              elements ++;
              if(elements>slots){
                 arr = realloc(arr,sizeof(char*)*elements);
                 if(arr==NULL) goto cleanup;
              }
              arr[elements-1] = (char*)temp;
            }
            /*skip sep*/
            start_index = end_index+sepLength+1;
            end_index = start_index;
        }
        pos++;
     }
     *count = elements;
     return arr;
cleanup:{
            int i=0;
            for(i=0;i<elements;i++)
              free(arr[i]);
            free(arr);
            *count =0;
            return NULL;
        }
}

/*对输出进行过滤 看是否含有history 或者history -i命令*/
static void filter(char *str){
    if(str && str[0]!='\0'){
        int count=0;
        char **p;
        char str_copy[1024] = {0};
        strcpy(str_copy,str);
        p = explode(str," ",&count);
        if(count>0){
            if(count==1 && strcmp(p[0],"history")==0){
                print_r(&history,-1);
                appendCommandToHistory(str_copy);
                init();
            }
            if(count==2 && strcmp(p[0],"history")==0){
                int len= strlen(p[1]);
                if(p[1][0]=='-' && len>=2){
                    /*判断后面的都是数字*/
                    int i=0;
                    int flag = 0;
                    for(i=1;i<len;i++){
                        if(p[1][i]>='0' && p[1][i]<='9')
                          flag =1;
                        else
                          flag =0;
                    }
                    if(!flag){ //格式不对
                        char *error="\nerror: format is unexcpted!";
                        write(l.ofd,error,strlen(error));
                        init();
                        appendCommandToHistory(str_copy);
                    }else{
                        char *temp;
                        temp = print_r(&history,atoi(p[1]+1));
                        if(temp!=NULL){
                            strcpy(l.buf,temp);
                            l.buf_length = strlen(temp);
                            appendCommandToHistory(str_copy);
                        }else{
                          l.buf[0]='\0';
                          l.buf_length=0;
                        }
                    }
                }
            }
        }
    }
}

/**
 * up ^[A
 * down ^[B
 * right ^[C
 * left ^[D
 * home ^[1~
 * end ^[4~
 *
 * delete ^[3~
 */
char *cli_run(const char *prompt){
     init();
     set_termi_attr();
     char c;
     int bytes;
     char err[64] = {0};
     char seq[5] = {0}; //保存输入序列
     setPrompt(prompt);
     refreshLine(&l);
     while(1){
          bytes = read(l.ifd,&c,1);
          vsnlogs(LOG_LEVEL_WARNING,"char=%c,charcode=%d,lenght=%d",c,c,bytes);

          if(bytes<0){
           snprintf(err,sizeof(err),"file=%s,line=%d,errstr=%s",__FILE__,__LINE__,strerror(errno));
           logs(LOG_LEVEL_WARNING,err,1);
          }
          switch(c){
           case KEY_ENTER:
                tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                filter(l.buf);
                return strdup(l.buf);
                break;
           case KEY_CTRL_A:
                cursorMoveToHome(&l);
                break;
           case KEY_TAB:
                if ( tab_handle != NULL )
                    tab_handle(l.buf);
                refreshLine(&l);
                break;
           case KEY_CTRL_C:
                //cursorMoveCenter(&l);
           		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                saveHistoryCmd(&history);
                printf("\n");
                return NULL;
                break;
           case KEY_CTRL_E:
                cursorMoveToEnd(&l);
                break;
           case KEY_CTRL_D:
                deleteLine(&l);
                break;
           case KEY_BACKSPACE:
                lineDeleteChar(&l);
                break;
           case KEY_ESC:
                if(read(l.ifd,seq,2)<0) vsnlogs(LOG_LEVEL_WARNING,"file=%s,line=%d,errstr=%s",__FILE__,__LINE__,strerror(errno));
                if(seq[0]=='['){
                    if(seq[1]>='0' && seq[1]<='9'){
                        read(l.ifd,seq+2,1);
                        if(seq[1]=='3' && seq[2]=='~'){ /*按下了delete key*/
                            deleteLine(&l);
                            vsnlogs(LOG_LEVEL_WARNING,"delete key pressed");
                        }
                        if(seq[1]=='1' && seq[2]=='~'){ /*按下home键*/
                            cursorMoveToHome(&l);
                            vsnlogs(LOG_LEVEL_WARNING,"home key pressed");
                        }
                        if(seq[1]=='4' && seq[2]=='~'){ /*按下end键*/
                            cursorMoveToEnd(&l);
                            vsnlogs(LOG_LEVEL_WARNING,"end key pressed");
                        }
                    //判断是否了方向键
                    }else{
                        switch(seq[1]){
                            case 'A':
                                  keyUpHandle(&l);
                                  break;
                            case 'B':
                                  keyDownHandle(&l);
                                  break;
                            case 'C':
                                  cursorMoveRight(&l);
                                  break;
                            case 'D':
                                  cursorMoveLeft(&l);
                                  break;
                            default:
                                  break;
                        }
                    }
                }else{
                  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                  saveHistoryCmd(&history);
                  printf("\n");
                  return NULL;
                }
                break;
           default :
            lineInsertChar(&l,c);
            vsnlogs(LOG_LEVEL_WARNING,"char=%d,l.buf_length=%d,l.buf=%s,l.pos=%d,%d+%d=%d",
              c,l.buf_length,l.buf,l.pos,l.prompt_length,l.pos,l.prompt_length+l.pos);
            break;
          }
     }
}

/* 最多20个参数 */
#define PARAM_MAX                     (20)

/* 全局变量，命令行节点头*/
struct NODE_CALLBACK *g_cli_head = NULL;

char g_input_cmd[1024];

/*
    创建新节点
*/
struct NODE_CALLBACK *cb_node_new(char *name)
{
    unsigned int len = 0;
    struct NODE_CALLBACK *node_new = NULL;

    if ( name == NULL )
        return NULL;

    node_new = malloc(sizeof(struct NODE_CALLBACK));
    if ( node_new == NULL )
        return NULL;
	
	memset(node_new, 0, sizeof(struct NODE_CALLBACK));
	
    len = strlen(name);
    node_new->name = malloc(len+1);
    if ( node_new->name == NULL )
    {
        free(node_new);
        return NULL;
    }
	
	memset(node_new->name , 0, len+1);
    strcpy(node_new->name, name);
    node_new->cb = NULL;
    node_new->bro = NULL;
    node_new->son = NULL;

    return node_new;
}

/*
    销毁节点
*/
void cb_node_free(struct NODE_CALLBACK *node)
{
    if ( node == NULL ) 
        return;

    if ( node->name != NULL )
        free(node->name);

    free(node);

    return;
}

/*
    销毁所有节点
*/
void cb_node_free_all(struct NODE_CALLBACK *pnode)
{
    if ( pnode == NULL )
        return;

    if ( pnode->bro != NULL )
        return cb_node_free_all(pnode->bro);
    else if ( pnode->son != NULL )
        return cb_node_free_all(pnode->son);
    else
        cb_node_free(pnode);

    return;
}

/*
    增加兄弟节点
*/
int cb_node_add_brother(struct NODE_CALLBACK *old, struct NODE_CALLBACK *young)
{
    if ( (old == NULL) || (old->bro != NULL) || (young == NULL) )
        return -1;

    old->bro = young;

    return 0;
}

/*
    增加子节点
*/
int cb_node_add_son(struct NODE_CALLBACK *father, struct NODE_CALLBACK *son)
{
    if ( (father == NULL) || (father->son != NULL) || (son == NULL) )
        return -1;

    father->son = son;

    return 0;
}

/* 
    兄弟维度找节点
*/
struct NODE_CALLBACK *cb_node_find_brother(struct NODE_CALLBACK *pnode, char *name)
{
    struct NODE_CALLBACK *p = pnode;

    if ( (pnode == NULL) || (name == NULL) )
        return NULL;

    while ( p )
    {
        if ( strcmp(p->name, name) == 0 )
            return p;
        else
            p = p->bro;
    }

    return NULL;
}

/*
把字符串转用参数
*/
int cb_func_str2params(char *str, int *num, char **argv)
{
    int argc;
    char *p = str;

    if ( (p == NULL) || (num == NULL) || (argv == NULL) )
        return -1;

    strcpy(g_input_cmd, str);
    p = g_input_cmd;

    /* 格式化字符串，把空格分开的词分成多个字符串 */
    while ( *p && (' ' == *p) ) p++; /* 忽略前面的空格 */

    if ( *p == '\0' )
        return -1;

    argc = 0;
    argv[argc++] = p;

    while ( *p )
    {
        if ( ' ' == *p ) 
        {
            *p++ = '\0';

            while ( *p && (' ' == *p) ) p++;

            if ( *p == '\0' )
                break;
            else
                argv[argc++] = p;

            if ( argc >= PARAM_MAX )
                return -1;
        }

        p++;
    }

    *num = argc;

    return 0;
}

/*
    注册回调函数
*/
int cb_func_reg(char *cmd, pFUNC_CALLBACK pFunc)
{
    int argc;
    char *argv[PARAM_MAX];
    int idx;
    struct NODE_CALLBACK *tmp_node = NULL;
    struct NODE_CALLBACK *pnode = g_cli_head;

    if ( cmd == NULL )
        return -1;

    if ( cb_func_str2params(cmd, &argc, argv) != 0 )
        return -1;

    idx = 0;
    pnode = g_cli_head;
    while ( idx < argc )
    {
        /* 找兄弟节点 */
        tmp_node = pnode;
        while ( tmp_node )
        {
            if ( strcmp(tmp_node->name, argv[idx]) == 0 )
                break;
            else
                tmp_node = tmp_node->bro;
        }

        if ( tmp_node == NULL ) 
        {/* 没找到，创建一个兄弟节点并指向它 */
            tmp_node = cb_node_new(argv[idx]);
            if ( tmp_node == NULL )
            return -1;

            while ( pnode->bro != NULL )
                pnode = pnode->bro;

            pnode->bro = tmp_node;
            pnode = pnode->bro;
            idx++;
        }
        else
        {/* 找到了，就指向它 */
            pnode = tmp_node;
            idx++;
        }

        /* 后面还有参数就指向子节点，没有子节点就创建一个新的子节点 */
        if ( idx < argc )
        {
            if ( pnode->son == NULL )
            {
                tmp_node = cb_node_new(argv[idx]);
                if ( tmp_node == NULL )
                    return -1;
                
                pnode->son = tmp_node;
                pnode = pnode->son;
            }
            else
            {
                pnode = pnode->son;
            }
        }
    }

    /* 指定回调函数 */
    pnode->cb = pFunc;
    
    return 0;
}

/*
    根据命令行找回调函数
*/
struct NODE_CALLBACK *cb_func_find(int argc, char **argv)
{
    int idx;
    struct NODE_CALLBACK *tmp_node = NULL;
    struct NODE_CALLBACK *pnode = g_cli_head;
    struct NODE_CALLBACK *pnode_find = NULL;  /* 找到的目标节点 */

    if ( (argc == 0) || (argv == NULL) )
        return NULL;

    idx = 0;
    pnode = g_cli_head;
    while ( idx < argc )
    {      
        /* 找兄弟节点 */
        tmp_node = pnode;
        while ( tmp_node )
        {
            if ( strcmp(tmp_node->name, argv[idx]) == 0 )
                break;
            else
                tmp_node = tmp_node->bro;
        }

        /* 找不到就跳出 */
        if ( tmp_node == NULL ) 
            break;
        else
            pnode = tmp_node;

        pnode_find = pnode;

        /* 指向子节点 */
        if ( pnode->son == NULL )
            break;
        else
            pnode = pnode->son;

        idx++;
    }

    return pnode_find;
}

/*
    按下table键的处理，把所有兄弟节点打印出来
*/
void cb_tab_print(char *line_text)
{
    int argc;
    char *argv[PARAM_MAX];
    char print_buf[1024] = {0};
    char tmp_buf[1024] = {0};
    struct NODE_CALLBACK *tmp_node = NULL;
    struct NODE_CALLBACK *pnode = g_cli_head;

    //printf("\r\nthis is table callback!\r\n");

    if ( line_text == NULL )
        return;

    if ( cb_func_str2params(line_text, &argc, argv) != 0 )
        return;

    /* 如果能找到完整的词就列出所有子节点，否则列出补全的节点 */
    pnode = cb_func_find(argc, argv);
    if ( pnode && pnode->son )
    {
        pnode = pnode->son;
        tmp_node = pnode;

        while ( tmp_node )
        {
            if ( strncmp(tmp_node->name, argv[argc-1], strlen(argv[argc-1])) == 0 )
            {
                sprintf(tmp_buf, "%s ", tmp_node->name);
                strcat(print_buf, tmp_buf);
            }
            tmp_node = tmp_node->bro;
        }

        if ( strlen(print_buf) == 0 )
        {
            while ( pnode )
            {
                sprintf(tmp_buf, "%s ", pnode->name);
                strcat(print_buf, tmp_buf);
                pnode = pnode->bro;
            }
        }
    }

    printf("\r\n%s\r\n", print_buf);
}

int ver(int argc, char **argv)
{
    printf("version is 1.0\r\n");
    return 0;
}

int bmcu_reg_read(int argc, char **argv)
{/* "bmcu reg read [addr]" */
    if ( argc == 4 )
        printf("read reg[%s] = 0x55\r\n", argv[3]);
    return 0;
}

int bmcu_reg_write(int argc, char **argv)
{/* "bmcu reg write [addr] [val]" */
    if ( argc == 5 )
        printf("write reg[%s] = %s\r\n", argv[3], argv[4]);
    return 0;
}

void cli_init(void)
{
    /* 头节点初始化 */
    g_cli_head = cb_node_new("head");
    if ( g_cli_head == NULL )
    {
        printf("creat cli head fail!\r\n");
        return;
    }
    
    tab_handle_set(cb_tab_print);
    
    loadHistoryCmd(HISTORY_FILE_PATH); /*加载历史命令*/
}

/**API
 * *void loadHistoryCmd(char *history_file_path);
 * *int  appendCommandToHistory(char *cmd);
 * *char *cli_run(const char*prompt);
 * */
 
//int main(int argc, char **argv)
int run_main(char *prompt)
{
    char *lineText;
    //char *prompt = (char *)"bsp>";        /*提示字符串*/
    if(!prompt)
        prompt = (char *)"bsp>";
    struct NODE_CALLBACK *pcb = NULL;
    int argc;
    char *argv[PARAM_MAX];

    while((lineText=cli_run(prompt))!=NULL)
    {
        if(lineText[0]!='\0')
        {
            printf("\n");

            if ( strcmp("quit", lineText) == 0 )
                break;

            if ( cb_func_str2params(lineText, &argc, argv) == 0 )
                if ( (pcb = cb_func_find(argc, argv)) != NULL )
                    if ( pcb->cb != NULL )
                        pcb->cb(argc, argv);

            appendCommandToHistory(lineText);  /*添加一条命令*/
        }
        else
        {
            printf("\n");
        }
    }

    cb_node_free_all(g_cli_head);

    return 0;
}

