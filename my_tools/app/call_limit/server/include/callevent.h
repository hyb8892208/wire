#ifndef _CALLEVENT_H
#define _CALLEVENT_H



#define   EVENT_LABEL           "Event: "
#define   EVENT_LABEL_LEN       7
#define   CHANNEL_LABEL         "Channel: "
#define   CHANNEL_LABEL_LEB     9
#define   RESULT_LABEL          "Result: "
#define   RESULT_LABEL_LEN  8
#define   SMS_STATUS_LABEL        "Status: "
#define   SMS_STATUS_LABEL_LEN 8
#define   SMS_DES_LABEL "Destination: "
#define   SMS_DES_LABEL_LEN  13
#define   SMS_REPORT_SENDER "Sender: "
#define   SMS_REPORT_SENDER_LEN 8

#define   CALL_STR_LEN           40
#define   READ_BUFF_LEN          4096      
#define   WRITE_BUFF_LEN         128   

#define   EVENT_TIMEOUTMS       1000

#define   STR_DIAL               "ExtraOutDial"
#define   STR_RING               "ExtraRingBack"
#define   STR_ANSWER             "ExtraConnect"
#define   STR_HANGUP             "ExtraHangup"
#define   STR_INDIAL             "ExtraDial"
 
#define   STR_SIMIN              "SimInserted"
#define   STR_SIMOUT             "ExtraDown"
#define   STR_READY           	  "ExtraReady"
#define   STR_SMS                "SMSSendStatus"
#define   STR_SMS_REPORT      "SMSStatusReport"
#define   STR_RESTART         "FullyBooted"
#define   STR_NO_CARRIER     "no_carrier"

#define   SERVER_IP              "127.0.0.1"
#define   SERVER_PORT            5038

#define   LOGIN_STR              "Action:Login\nUsername:event\nSecret:event\n\n"


extern int event_thread_create(void);
extern call_status_t str_to_call_status(char *call_sta);
extern const char * call_status_to_str(call_status_t call_status);



#endif

