#ifndef __SWG_DEVICE_H_
#define __SWG_DEVICE_H_
        
/*   
 *       #############################################          
 *       #    SWG (Standalone wireless Gateway)      # 
 *       #    Device Hardware Structure Diagram      # 
 *       #############################################          
 *                                                              
 *       +-------------------------------------------+          
 *       |               SWG Device                  |          
 *       |                                           |          
 *   +---|- +---------------+     +---------------+  |          
 *   |   |  | Module Board 0| ... | Module Board n|  |          
 *   |   |  +---------------+     +---------------+  |          
 *   |   |                                           |          
 *   |   +-------------------------------------------+          
 *   |                                                          
 *   |                                                          
 *   +-> +-------------------------------------------+          
 *       |              Module Board                 |          
 *       |                                           |          
 *       |          +------------------+             |          
 *       |          |    Board  MCU    |             |          
 *       |          +------------------+             |          
 *       |                                           |          
 *   +---|-+----------------+     +----------------+ |          
 *   |   | | Channel Board 0| ... | Channel Board n| |          
 *   |   | +----------------+     +----------------+ |          
 *   |   |  Note:                                    |          
 *   |   |  Board MCU control Channel Board 0 to n   |
 *   |   |  each channel Power ON/OFF, Check SIM     |          
 *   |   |  status etc.                              |          
 *   |   +-------------------------------------------+          
 *   |                                                          
 *   |                                                          
 *   +-> +-------------------------------------------+
 *       |              Channel Board                |          
 *       |     (MCU,By now only has 2 channels)      |          
 *       |                                           |          
 *       |   +--------------+     +--------------+   |          
 *       |   |   channel 0  |     |   channel n  |   |          
 *       |   |(m35/SIM6320C)| ... |(m35/SIM6320C)|   |          
 *       |   +--------------+     +--------------+   |          
 *       +-------------------------------------------+          
 */   

#include <pthread.h> /* for pthread_mutex_t */
#include <sys/time.h> /*for struct timeval*/
#include "mod_brd_hw_intf.h" /* for struct hw_intf */

/* 最多支持通道数 */
/* 目前2U机箱最多44路通道 */
#define     MAX_CHAN_NUM            (44)

/* 模块板最多MCU个数, 个数与机箱槽位线印一致，没有槽位的从0开始顺序编号。
 * 目前2U机箱槽位最多(12个, 插主板的槽位也算)*/
#define MAX_MOD_BRD_NUM             (12) 

/* 1个模块板MCU控制最多通道数 */
/* 目前16口无线网关1个模块板MCU控制16路通道 */
#define MAX_CHAN_NUM_PER_MCU        (16)
#define MAX_CHAN_NUM_PER_VS_USB_MCU	(4)

/* 1个模块板最多通道板数 */
/* 目前16口无线网关1个模块板8个通道板 */
#define MAX_CHAN_BRD_PER_MOD_BRD    (8)

/* 1通道板上最多通道数 */
/* 目前每个通道板上都是2个通道 */
#define CHAN_NUM_PER_CHAN_BRD       (2)

#define SIM_SWITCH_NUM (2)
/* 通道板最大数 */
#define MAX_CHAN_BRD_NUM    (MAX_CHAN_NUM / CHAN_NUM_PER_CHAN_BRD)

/* 模块板(控制MCU)设备路径 */
#define MOD_BRD_PATH            "/dev/opvx/mod_brd/"

/*  通道板设备路径, 目前一个通道板控制2个通道 */
#define CHAN_BRD_PATH           "/dev/opvx/chan_brd/"

/*  EMU设备路径 */
#define EMU_PATH                "/dev/opvx/emu/"

/*  LCD设备路径 */
#define LCD_PATH                "/dev/opvx/lcd/"

/*  upgrade设备路径 */
#define UPGRADE_PATH            "/dev/opvx/upgrade/"

/*sim switch info*/
#define SIM_SWITCH_PATH "/dev/opvx/sim_switch/"

/* SWG设备硬件信息文件 */
#define SWG_HW_INFO_FILE         "/tmp/hw_info.cfg"
/* SWG设备所有通道类型文件, 历史原因命名不当*/
#define SWG_CHAN_TYPE_FILE       "/tmp/.module_type"
/* SWG设备通道总数文件, 历史原因命名不当*/
#define SWG_TOTAL_CHAN_FILE      "/tmp/.boardtype"
//设备名称长度
#define DEV_NAME_LEN		32
typedef enum mod_brd_name_e
{
    MB_NAME_UNKOWN = 0,
    MB_SWG_1004_BASE,   /* 4口模块板, 目前无此硬件 */
    MB_SWG_1008_BASE,   /* 8口模块板, 目前无此硬件 */
    MB_SWG_1016_BASE,   /* 16口模块板,实际上目前16口与32口一样, 都是MB_SWG_1032_BASE, 此值预留 */
    MB_SWG_1032_BASE,   /* 32口模块板，虽然命令为1032,但硬件上只有16口，32口由2个此模块叠加而得，目前已有硬件 */ 
    MB_SWG_VS_USB_BASE, /* 可插拔4口USB模块*/
	MB_SWG_VS2_USB_DOWN,
}MOD_BRD_NAME_E;

typedef enum swg_name_e
{
    SWG_DEV_UNKOWN = 0,
    SWG_DEV_1001,       /* 1口无线网关 */
    SWG_DEV_1002,       /* 2口无线网关 */
    SWG_DEV_1004,       /* 4口无线网关 */
    SWG_DEV_1008,       /* 8口无线网关 */
    SWG_DEV_1016,       /* 16口无线网关 */ 
    SWG_DEV_1032,       /* 32口无线网关 */ 
    SWG_DEV_1064,       /* 64口无线网关 */ 
    SWG_DEV_VS_USB_1020,/* 20口可插拔USB模块无线网关 1U机箱*/
    SWG_DEV_VS_USB_1044,/* 44口可插拔USB模块无线网关 2U机箱*/
    SWG_DEV_VS_USB_1008,	/* VS_USB 1008 or 1004 */
	SWG_DEV_VS2_8X_1064,    /* VS2_8X 1064 */
}SWG_NAME_E;

typedef enum _sys_type_e_
{
	SYS_TYPE_UNKOWN = 0,
	SYS_TYPE_SWG20XX,
	SYS_TYPE_1CHAN4SIMS,
	SYS_TYPE_VS_USB,
	SYS_TYPE_VS2_USB,
	SYS_TYPE_M20X,
/*	SYS_TYPE_VS_1008,*/
}SYS_TYPE_E;

/* a - major, b - minor, c - bugfix */
#define VERSION_NUMBER(a,b,c) (((a) << 16) + ((b) << 8) + (c))
typedef unsigned int ver_type_t;

#define GET_VERSION_MAJOR(VERSION)          (((VERSION)>> 16) & 0xFF)
#define GET_VERSION_MINOR(VERSION)          (((VERSION)>> 8) & 0xFF)
#define GET_VERSION_BUGFIX(VERSION)         (((VERSION)) & 0xFF)

/* 定义系统硬件型号类型, 
 * 硬件类型由硬件名称和硬件版本号共同决定, 参考宏:HW_TYPE(NAME, HW_VER)*/
typedef unsigned long  hw_type_t;

/* 定义模块板硬件型号
 * 目前模块板硬件型号，即系统硬件型号 */
typedef unsigned long  brd_type_t;

/* 定义通道（无线）模块类型 */
typedef enum chan_mod_type_e
{
    CHAN_MOD_TYPE_UNKOWN = 0,
    CHAN_MOD_M35,               /* M35模块(GSM) */
    CHAN_MOD_SIM6320C,          /* sim6320c(CDMA)*/
    CHAN_MOD_EC20F,              /* EC20F(全网通)*/
	CHAN_MOD_UC15,              /* UC15模块(3G)*/
	CHAN_MOD_EC25E,             /* EC25E(全网通)*/
	CHAN_MOD_M26,               /* M26模块(GSM) */
    CHAN_MOD_TYPE_NUM,
}CHAN_MOD_TYPE_E;

/* 定义模块板状态 */
typedef enum mod_brd_status_e
{
    MB_STATUS_NOT_INSERT = 0,   /* 未插入 */
    MB_STATUS_INSERT,           /* 已插入 */
    MB_STATUS_INIT_FAILED,      /* 初始化失败 */
    MB_STATUS_INITED,           /* 初始化完成，可正常工作 */
} MOD_BRD_STATUS_E;

typedef enum chan_mod_led_name_e{
    CHAN_MOD_LED_SIG = 0,
    CHAN_MOD_LED_WORK,
    CHAN_MOD_LED_NUM,
}CHAN_MOD_LED_NAME_E;

typedef enum chan_mod_lamp_status_e
{
    CHAN_LAMP_STATUS_OFF = 0,       
    CHAN_LAMP_STATUS_GREEN_ON,         
    CHAN_LAMP_STATUS_GREEN_FLASH,     
    CHAN_LAMP_STATUS_RED_ON,
    CHAN_LAMP_STATUS_RED_FLASH,
    CHNA_LAMP_STATUS_YELLOW_ON,
    CHNA_LAMP_STATUS_YELLOW_FLASH,
    CHAN_LAMP_STATUS_MAX,
} CHAN_MOD_LAMP_STATUS_E;


typedef enum mod_brd_led_name_e{
	MOD_BRD_LED_SYS = 0,
	MOD_BRD_LED_NUM,
}MOD_BRD_LED_NAME_E;

typedef enum MOD_BRD_LAMP_STATUS{
	MOD_BRD_LAMP_STATUS_OFF = 0,
	MOD_BRD_LAMP_STATUS_GREEN_ON,
	MOD_BRD_LAMP_STATUS_GREEN_FLASH,
}MOD_BRD_LAMP_STATUS;

/* 定义模块板状态 */
typedef enum chan_brd_status_e
{
    CB_STATUS_NOT_INSERT = 0,   /* 未插入 */
    CB_STATUS_INSERT,           /* 已插入 */
    CB_STATUS_INIT_FAILED,      /* 初始化失败 */
    CB_STATUS_INITED,           /* 初始化完成，可正常工作 */
} CHAN_BRD_STATUS_E;

/* sim card 事件描述 */
typedef enum sim_card_evt_e
{
    SIMCARD_EVENT_UNKOWN = 0, /* 未知 */
    SIMCARD_EVENT_INSERT,     /* 插入事件 */
    SIMCARD_EVENT_REMOVE      /* 拔出事件 */
} SIM_CARD_EVT_E;

typedef enum chan_mod_power_event_e{
	CHAN_MOD_POWER_OFF = 0,
	CHAN_MOD_POWER_ON,
	CHAN_MOD_IS_POWER_ON,
	CHAN_MOD_IS_POWER_OFF,
	CHAN_MOD_POWER_RESET,
	CHAN_MOD_POWER_UNKOWN,
}CHAN_MOD_POWER_EVENT_E;

typedef enum chan_mod_vbat_event_e{
	CHAN_MOD_VBAT_OFF = 0,
	CHAN_MOD_VBAT_ON,
	CHAN_MOD_VBAT_UNKOWN,
}CHAN_MOD_VBAT_EVENT_E;
typedef enum chan_mod_simcard_event_e{
	CHAN_MOD_SIM_DISABLE = 0,
	CHAN_MOD_SIM_ENABLE,
	CHAN_MOD_SIM_UNKOWN,
}CHAN_MOD_SIMCARD_EVENT_E;

typedef enum chan_mod_powerkey_event_e{
	CHAN_MOD_POWERKEY_LOW=0,
	CHAN_MOD_POWERKEY_HIGH,
	CHAN_MOD_POWERKEY_UNKOWN,
}CHAN_MOD_POWERKEY_EVENT_E;

typedef enum chan_upgrade_status_e
{
    NOT_UPGRADE_STATUS = 0,      /* 不是升级状态 */
    IS_UPGRADE_STATUS,         /* 升级状态中，不可进行模块操作 */
}CHAN_UPGRADE_STATUS_E;

typedef enum chan_mod_event{
	CHAN_MOD_EVENT = 0,
	CHAN_MOD_EVENT_NUM,
}CHAN_MOD_EVENT_E;

struct mod_brd_sched {
	struct timeval when;
	int delay;
	void (*callback)(void *data);
	void *data;
};

typedef enum chan_brd_protocol_type_e{	
	CHAN_BRD_PROTOCOL_UNKOWN = 0,
	CHAN_BRD_PROTOCOL_HDLC = 1,
	CHAN_BRD_PROTOCOL_COBS = 2,
	CHAN_BRD_PROTOCOL_NUM,
}CHAN_MOD_PROTOL_TYPE_E;


typedef struct sim_slot_e{
     SIM_CARD_EVT_E         sim_evt;        /* sim event */
     SIM_CARD_EVT_E         sim_prv_evt;    /* sim previous event, 上次上报的sim卡事件 */
}SIM_SLOT_S;


typedef enum sim_slot {
	SIMCARD_SOLT_1 = 0,
	SIMCARD_SOLT_2,
	SIMCARD_SOLT_3,
	SIMCARD_SOLT_4,
}SIM_CARD_SOLT_E;

typedef enum sim_switch_status_e{
    SIM_SWTICH_STATUS_NOT_INSERT = 0,   /* 未插入 */
    SIM_SWTICH_STATUS_INSERT,           /* 已插入 */
    SIM_SWTICH_STATUS_INIT_FAILED,      /* 初始化失败 */
    SIM_SWTICH_STATUS_INITED,           /* 初始化完成，可正常工作 */
}SIM_SWITCH_STATUS_E;

typedef struct sim_channel{
    SIM_CARD_SOLT_E        cur_slot; 
    SIM_SLOT_S            sim_slot[4];
    struct sim_device *sim;
}SIM_CHANNEL_S;

/* 通道描述 */
typedef struct swg_channel
{
    CHAN_MOD_TYPE_E         mod_type;       /* 通道通信模块类型 */
    int                     chan_no;        /* 通道编号, 即在/dev/opvx/channel/下的设备名 */
    CHAN_UPGRADE_STATUS_E    upgrade_flag;    /* 通道升级标记位 */
    struct  swg_mod_brd     *mod_brd;       /* 通道所在模块板 */
    struct  swg_chan_brd    *chan_brd;      /* 通道所在通道板 */
    struct sim_card_ops     *sim_ops;       /* sim card operations */
     SIM_CARD_EVT_E         sim_evt;        /* sim event */
     SIM_CARD_EVT_E         sim_prv_evt;    /* sim previous event, 上次上报的sim卡事件 */
	struct timespec         timeout;
	int                     chan_power_on_off_try_cnt;  /*模块开关机尝试次数统计*/
	int                     chan_power_on_off_delay;    /*模块开关机睡眠时间*/
	int                     chan_power_on_off_duration; /*模块开关机时间*/
} SWG_CHANNEL_ST;

/* 通道板(SWG channel module)描述 */
typedef struct swg_chan_brd
{
    /* 硬件信息 */
    //brd_type_t  brd_type;           [> 硬件类型 <]
    ver_type_t  hw_ver;             /* 硬件版本号 */
    ver_type_t  sw_ver;             /* 软件版本号 */
    int       index;                    /* 通道板编号，即在/dev/opvx/chan_board/ */
    int       channels;                 /* 该通道板上总共通道数 */
    int       base_chan_no;             /* 该通道板上在系统上的起始通道编号 */
    CHAN_BRD_STATUS_E    status;        /* 状态信息，0代表不可用，1代表可用 */
	CHAN_MOD_PROTOL_TYPE_E protocol;
} SWG_CHAN_BRD_ST;

typedef struct sim_device{
   struct hw_intf               hwif;
   struct swg_device          *dev;
   unsigned char                sim_slot_reg_map[4];
 //  SIM_CHANNEL_S           sim_chn[MAX_CHAN];
   pthread_mutex_t           sim_lock;
   enum sim_switch_status_e state;
   ver_type_t hw_ver;
   ver_type_t sw_ver;
   int index;
   int channels;
   int base_chan_no;
}SIM_DEVICE_S;


/* 模块板(SWG module board)描述 */
typedef struct swg_mod_brd
{
    /* 硬件信息 */
    MOD_BRD_NAME_E  name;           /* 设备名称 */
    ver_type_t  hw_ver;             /* 硬件版本号 */
    ver_type_t  sw_ver;             /* 软件版本号 */
    int         index;              /* mcu编号，即在/dev/opvx/mcu/下的设备名 */
    int         channels;           /* 该mcu模块板总共通道数 */
    int         base_chan_no;       /* 该mcu模块板起始通道编号 */
    struct swg_device   *swg_dev;   /* 模块板所在设备 */

    /* 软件信息 */
    struct hw_intf      hwif;
    MOD_BRD_STATUS_E    status;        /* 状态信息 */
	MOD_BRD_LAMP_STATUS sys_lamp_status;
	CHAN_MOD_LAMP_STATUS_E    reg_lamp_status[MAX_CHAN_NUM_PER_VS_USB_MCU];	/* 注册灯状态 */
	CHAN_MOD_LAMP_STATUS_E    call_lamp_status[MAX_CHAN_NUM_PER_VS_USB_MCU];	/* 呼叫灯状态 */
	CHAN_MOD_SIMCARD_EVENT_E  simcard_evet[MAX_CHAN_NUM_PER_MCU];/*需要设置的sim事件*/
	CHAN_MOD_VBAT_EVENT_E     vbat_event[MAX_CHAN_NUM_PER_MCU];/*需要设置的vbat事件*/
	CHAN_MOD_POWER_EVENT_E    power_event[MAX_CHAN_NUM_PER_MCU];/*需要设置的power事件*/
	CHAN_MOD_POWER_EVENT_E    power_next_event[MAX_CHAN_NUM_PER_MCU];/*下次需要设置的power事件，用于模块重启*/
	CHAN_MOD_POWERKEY_EVENT_E powerkey_event[MAX_CHAN_NUM_PER_MCU];/*需要设置的powerkey事件*/
	unsigned char             simcard_enable_status_l;/*模块板1-8通道sim卡enable/disable状态*/
	unsigned char             simcard_enable_status_h;/*模块板9-16通道sim卡enable/disable状态*/
	unsigned char             vbat_status_l;/*模块板1-8通道供电/断电状态*/
	unsigned char             vbat_status_h;/*模块板9-16通道供电/断电状态*/
	unsigned char             simcard_insert_status_l;/*模块板1-8通道sim卡在位/不在位状态*/
	unsigned char             simcard_insert_status_h;/*模块板9-16通道sim卡在位/不在位状态*/
	unsigned char             powerkey_status_l;/*模块板1-8通道powerkey电平状态*/
	unsigned char             powerkey_status_h;/*模块板9-16通道powerkey电平状态*/
	unsigned char             power_status_l;/*模块板1-8通道开机/关机状态*/
	unsigned char             power_status_h;/*模块板9-16通道开机/关机状态*/
	unsigned char             lamp_flash;           /*led灯状态*/
	unsigned char             sys_lamp_flash;       /*system led 灯状态*/
	struct mod_brd_sched mod_sched[CHAN_MOD_EVENT_NUM];
	pthread_t handler_id;
	pthread_mutex_t     lock;           /* 锁，串行访问此设备 */
 
} SWG_MOD_BRD_ST;

/* 设备描述 */
typedef struct swg_device
{
    /* 硬件名称和硬件版本号共同确定一唯一设备 */
    SWG_NAME_E  name;               /* 设备名称 */
    SYS_TYPE_E sys_type;		/* system type */
    ver_type_t  version;            /* 版本号 */
    int         total_mod_brds;     /* 实际模块板数, 即最后一个可识别的模块板号(从0编号)加1*/
    int         total_chan_brds;    /* 实际通道板数, 即最后一个可识别的通道板号(从0编号)加1*/
    int         total_chans;        /* 实际总通道数, 即最后一个可识别的通道号(从0编号)加1*/
    int         total_sim_sws;
    struct swg_mod_brd      mod_brd[MAX_MOD_BRD_NUM];   /* 模块板 */
    struct swg_chan_brd     chan_brd[MAX_CHAN_BRD_NUM]; /* 通道模块板 */
    struct swg_channel       chans[MAX_CHAN_NUM];        /* 通道 */
    struct sim_channel        sim_chan[MAX_CHAN_NUM];  /*sim 卡槽*/
    struct sim_device         sim_switch[SIM_SWITCH_NUM];         /*sim 卡槽板*/
    pthread_t   dt_id;              /* detection thread ID, 此线程检测SWG设备硬件变化情况 */
    int         dtct_intv;          /* 此线程检测检测时间间隔，  单位ms */
    pthread_t   led_id;             /* led线程ID,此线程控制led灯闪烁*/
} SWG_DEV_ST;

struct sim_card_ops
{

    /*************************************************
      函数描述 : 模块紧急关机
      输入参数 : brd -- 模块板结构体
                 chan -- 指定紧急关机的通道, =0xFF表示所有通道
      函数返回 :  0 -- 成功
                 <0 -- 失败
     *************************************************/
    int (* emergency_power_off)(struct swg_mod_brd *brd, int chan);	
	int (* translate_vbat)(struct swg_mod_brd *mb, int md_idx, int *reg_l, int *reg_h);
	int (* translate_vbat_status)(struct swg_mod_brd *mb, int md_idx, int *reg_value);
	int (* translate_power_status)(struct swg_mod_brd *mb, int md_idx, int *reg_value);
};

/*
 *设备和槽位号
 *
 * */
typedef struct _mcu_dev_s
{
	char node_name[DEV_NAME_LEN];
	int slot_num;
}mcu_dev_t;
#endif // __SWG_DEVICE_H_
