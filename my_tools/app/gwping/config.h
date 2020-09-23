
#ifndef ETH_SVC_CONFIG_H_
#define ETH_SVC_CONFIG_H_

// OpenVox Service Over Ethernet Ethernet frame protocol type
#define _ETHER_TYPE_OPVX	    0xd01d  

#define _ETH_SEND_HEAD_FLAG      "OVSD"
#define _ETH_RECV_HEAD_FLAG      "OVRV"
#define _ETH_SVC_PING           "GWPing"

// service header
struct RawSvcHead {
    char flag[4];           // must be "OPVX"
    short size;             // message body size of this packet
    char unique_id[16];     // a 16 byte uuid identify this packet
    char body[0];           // start of the message body
} 
#ifndef WIN32
__attribute__ ((__packed__))
#endif
;

// uncomment this line to disabe assert
// #define NDEBUG


#define ESVC_ERR   __FILE__ ": ERR: "

#define ESVC_DBG   __FILE__ ": DBG: "

#define ESVC_INFO   __FILE__ ": INFO: "

#define ESVC_WARN   __FILE__ ": WARNNING: "

//#define esvc_log    printf
#define esvc_log


#define CLIENT_LIVE_TIME (1*60)    // client live max 1 minutes.

#define _PING_WAIT_TIME 10  // wait time of ping cmd 

#define _CMD_WAIT_TIME  30  // wait time of other cmd



#endif // ETH_SVC_CONFIG_H_
