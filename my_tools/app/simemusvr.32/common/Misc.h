

/****************************************************************************
* 版权信息：
* 系统名称：SimRdrSvr
* 文件名称：EmuRegisterList.h 
* 文件说明：Emu注册管理列表头文件
* 作    者：hlzheng 
* 版本信息：v1.0 
* 设计日期：
* 修改记录：
* 日    期		版    本		修改人 		修改摘要  
****************************************************************************/

/**************************** 条件编译选项和头文件 ****************************/
#ifndef __MISC_G2_H__
#define __MISC_G2_H__

#include <queue>

#define MAXHIDPORT  40
#define SIMLEDCTROLLERID  500

//print Color Code
#define RED_CODE  0x91
#define GREEN_CODE  0x92
#define BLACK_CODE  0x93

#define RESET "\033[0m"
#define BLACK "\033[30m" /* Black */
#define RED "\033[31m" /* Red */
#define GREEN "\033[32m" /* Green */
#define YELLOW "\033[33m" /* Yellow */
#define BLUE "\033[34m" /* Blue */
#define MAGENTA "\033[35m" /* Magenta */
#define CYAN "\033[36m" /* Cyan */
#define WHITE "\033[37m" /* White */
#define BOLDBLACK "\033[1m\033[30m" /* Bold Black */
#define BOLDRED "\033[1m\033[31m" /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m" /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m" /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m" /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m" /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m" /* Bold White */

#define SIMRDRHIDBUFLEN 320
#define SIMlEDCTLBUFLEN 64
#define SCANATRNUMBER 5
#define EXITDURATION 2
#define INVALID_HANDLE_VALUE  NULL

typedef std::queue<int> MsgQueue;
#endif
