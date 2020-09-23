
#include "soapH.h"
#include "ns.nsmap"
#include "callmonitor.h"
#include <pthread.h>
//#include "gsoap_api.h"

#define GSOAP_SERVER_IP "127.0.0.1"
#define GSOAP_MAX_THR 5
#define GSOAP_SERVER_PORT 8811
#define GSOAP_MAX_QUEUE 10
#define	GSOAP_BACK_LOG		100

typedef struct _gsoap_info_s
{
	int head;
	int tail;
	SOAP_SOCKET queue[GSOAP_MAX_QUEUE]; 
	pthread_mutex_t queue_lock;
	pthread_cond_t queue_cond;
}gsoap_info_t;

typedef struct _gsoap_params_s
{
        unsigned short gsoap_port;
        unsigned short count;
}gsoap_params_t;

//static gsoap_sap_t gsoap_sap;
static gsoap_info_t gsoap_info;
static struct soap server_soap;

static void *gsoap_process_queue(void*);
static int gsoap_enqueue(SOAP_SOCKET);
static SOAP_SOCKET gsoap_dequeue();


void gsoap_init()
{
	//memset(&gsoap_sap, 0, sizeof(gsoap_sap_t));
	memset(&gsoap_info, 0, sizeof(gsoap_info_t));
}

/*int gsoap_bind(gsoap_sap_t *sap)
{
	if (sap == NULL)
		return -1;
	memcpy(&gsoap_sap, sap, sizeof(gsoap_sap_t));

	return 0;
}*/
void *gsoap_main_thread_cb_handler(void *arg)
{
	int i;
	unsigned short port = 0;
//	unsigned short count = 0;
	gsoap_params_t *ptr_params = NULL;
	SOAP_SOCKET master_sk, slave_sk;
	struct soap *ptr_soap = &server_soap;
	gsoap_info_t *ptr_info = &gsoap_info;
	struct soap *soap_thr[GSOAP_MAX_THR]; // each thread needs a runtime context 
	pthread_t gsoap_tid[GSOAP_MAX_THR];

	if (arg != NULL)
	{
		ptr_params = (gsoap_params_t*)arg;
		port = ptr_params->gsoap_port;
	//	count = ptr_params->count;
	}

	if(port == 0)
		port = GSOAP_SERVER_PORT;
	soap_init(ptr_soap);
	ptr_soap->bind_flags |= SO_REUSEADDR;	
	master_sk = soap_bind(ptr_soap, GSOAP_SERVER_IP, port, GSOAP_BACK_LOG);
	if(!soap_valid_socket(master_sk))
	{
		soap_print_fault(ptr_soap, stderr);
		exit(1);
	}
	pthread_mutex_init(&ptr_info->queue_lock, NULL);
	pthread_cond_init(&ptr_info->queue_cond, NULL);
	for (i = 0; i < GSOAP_MAX_THR; i++)
	{
		soap_thr[i] = soap_copy(ptr_soap);
//		fprintf(stderr, "Starting thread %d\n", i);
		call_monitor_create_thread(&gsoap_tid[i], (void*)soap_thr[i], (void*(*)(void*))gsoap_process_queue);
	}

	for (;;)
	{
		slave_sk = soap_accept(ptr_soap);
		if (!soap_valid_socket(slave_sk))
		{
			if (ptr_soap->errnum)
			{
				soap_print_fault(ptr_soap, stderr);
				continue; // retry 
			}
			else
			{
				fprintf(stderr, "Server timed out\n");
				break;
			}
		}
		while (gsoap_enqueue(slave_sk) == SOAP_EOM)
			sleep(1);
	}
	for (i = 0; i < GSOAP_MAX_THR; i++)
	{
		while (gsoap_enqueue(SOAP_INVALID_SOCKET) == SOAP_EOM)
			sleep(1);
	}
	for (i = 0; i < GSOAP_MAX_THR; i++)
	{
		fprintf(stderr, "Waiting for thread %d to terminate... ", i);
		pthread_join(gsoap_tid[i], NULL);
		fprintf(stderr, "terminated\n");
		soap_done(soap_thr[i]);
		free(soap_thr[i]);
	} 
	pthread_mutex_destroy(&ptr_info->queue_lock); 
	pthread_cond_destroy(&ptr_info->queue_cond);
	soap_done(ptr_soap);

	return NULL;
} 

void *gsoap_process_queue(void *soap)
{
	struct soap *ptr_soap = (struct soap*)soap;
	for (;;)
	{
		ptr_soap->socket = gsoap_dequeue();
		if (!soap_valid_socket(ptr_soap->socket))
		{
//			gsoap_print_warn("%s call gsoap_dequeue failed!, socket=%d. \n", __FUNCTION__, ptr_soap->socket);
			break;
		}
		soap_serve(ptr_soap);
		soap_destroy(ptr_soap);
		soap_end(ptr_soap);
	}

	return NULL;
}

int gsoap_enqueue(SOAP_SOCKET sock)
{
	int status = SOAP_OK;
	int next;
	gsoap_info_t *ptr_info = &gsoap_info;

	pthread_mutex_lock(&ptr_info->queue_lock);
	next = ptr_info->tail + 1;
	if (next >= GSOAP_MAX_QUEUE)
		next = 0;
	if (next == ptr_info->head)
		status = SOAP_EOM;
	else
	{
		ptr_info->queue[ptr_info->tail] = sock;
		ptr_info->tail = next;
		pthread_cond_signal(&ptr_info->queue_cond);
	}
	pthread_mutex_unlock(&ptr_info->queue_lock);

	return status;
}

SOAP_SOCKET gsoap_dequeue()
{
	SOAP_SOCKET sock;
	gsoap_info_t *ptr_info = &gsoap_info;

	pthread_mutex_lock(&ptr_info->queue_lock);
	while (ptr_info->head == ptr_info->tail)
		pthread_cond_wait(&ptr_info->queue_cond, &ptr_info->queue_lock);
	sock = ptr_info->queue[ptr_info->head++];
	if (ptr_info->head >= GSOAP_MAX_QUEUE)
		ptr_info->head = 0;
	pthread_mutex_unlock(&ptr_info->queue_lock);

	return sock;
}


int gsoap_thread_create(void)
{
	pthread_t gsoap_tid;

	call_monitor_create_thread(&gsoap_tid, NULL, gsoap_main_thread_cb_handler);

	return 0;	
}

int ns__config_reload(struct soap *gsoap, int *result)
{
	config_reload();
	return 0;
}

int ns__get_chan_conf(struct soap *gsoap, int chan_id, struct ns__chan_config *conf)
{
	get_chan_conf(chan_id, (struct chan_config*)conf);
	return 0;
}

int ns__get_chan_data(struct soap *gsoap, int chan_id, struct ns__chan_data *data)
{
	get_chan_data(chan_id, (struct chan_data *)data);
	return 0;
}

int ns__flush_status(struct soap *gsoap, int *result)
{
	flush_status_to_file();
	return 0;
}