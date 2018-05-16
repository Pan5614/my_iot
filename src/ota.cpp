#include <cstdio>
#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <fcntl.h>  
#include <unistd.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/mii.h>
#include <linux/sockios.h>
#include <errno.h>
#include <arpa/inet.h>
#include <linux/ethtool.h>
#include <pthread.h>

#include "iot_import.h"
#include "iot_export.h"

#include "config.h"
#include "ota.h"

#define OTA_BUF_LEN             (5000)

#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
} while(0)
    


#define DBG_EN 0
#if (DBG_EN == 1)
#define debugf(fmt, args...) do {\
        printf("[OTA] DEBUG: %s(): ", __func__);\
        printf(fmt, ##args);\
 } while (0)
#else
#define debugf(fmt, args...)
#endif

#define errf(fmt, args...) do {\
        printf("[OTA] ERROR: @ %s(): ", __func__);\
        printf(fmt, ##args);\
} while (0)

#define inf(fmt, args...) do {\
	    printf("[OTA] INFO: %s(): ", __func__);\
	    printf(fmt, ##args);\
} while (0)

#define warnf(fmt, args...) do {\
	    printf("[OTA] WARN: %s(): ", __func__);\
	    printf(fmt, ##args);\
} while (0)


void *pclient;
char *msg_buf = NULL, *msg_readbuf = NULL;



void ota_pthread_create(void)
{
    int ret;
    pthread_t ota_tid;
    pthread_attr_t ota_attr;

    pthread_attr_init(&ota_attr);
    pthread_attr_setdetachstate(&ota_attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&ota_tid, &ota_attr, ota_thread, NULL);            //创建线程
    pthread_attr_destroy(&ota_attr);
    if(ret != 0) {
        errf("create ota thread errer!\n");
    }
    inf("had been creat ota thread ok!\n");
}

void iot_pthread_create(void)
{
    int ret;
    pthread_t ota_tid;
    pthread_attr_t ota_attr;
	
    pthread_attr_init(&ota_attr);
    pthread_attr_setdetachstate(&ota_attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&ota_tid, &ota_attr, iot_thread, NULL);            //创建线程
    pthread_attr_destroy(&ota_attr);
    if(ret != 0) {
        errf("create iot thread errer!\n");
    }
    inf("had been creat iot thread ok!\n");
}

void *iot_thread(void* arg)
{
	int rc = 0, msg_len, cnt = 0;
	iotx_mqtt_topic_info_t topic_msg;
	char msg_pub[128];

	/* Subscribe the specific topic */
    rc = IOT_MQTT_Subscribe(pclient, TOPIC_GET, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        rc = -1;
        goto do_exit;
    }else{
	EXAMPLE_TRACE("IOT_MQTT_Subscribe() success !, rc = %d", rc);
    }

    /* Subscribe the specific topic */
    rc = IOT_MQTT_Subscribe(pclient, TOPIC_DATA, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        rc = -1;
        goto do_exit;
    }else{
	EXAMPLE_TRACE("IOT_MQTT_Subscribe() success !, rc = %d", rc);
    }

    /* Initialize topic information */
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    strcpy(msg_pub, "message: hello! start!");

    topic_msg.qos = IOTX_MQTT_QOS1;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = msg_pub;
    topic_msg.payload_len = strlen(msg_pub);

    rc = IOT_MQTT_Publish(pclient, TOPIC_DATA, &topic_msg);
    EXAMPLE_TRACE("rc = IOT_MQTT_Publish() = %d", rc);

 
    do {
        /* Generate topic message */
        cnt++;
	
	msg_len = snprintf(msg_pub, sizeof(msg_pub), "{\"attr_name\":\"temperature\", \"attr_value\":\"%d\"}", cnt);
        if (msg_len < 0) {
            EXAMPLE_TRACE("Error occur! Exit program");
            rc = -1;
            break;
        }
	
	topic_msg.payload = msg_pub;
        topic_msg.payload_len = msg_len;

        rc = IOT_MQTT_Publish(pclient, TOPIC_DATA, &topic_msg);
        if (rc < 0) {
            EXAMPLE_TRACE("error occur when publish");
            rc = -1;
            break;
        }

        EXAMPLE_TRACE("packet-id=%u, publish topic msg=%s", (uint32_t)rc, msg_pub);

        /* handle the MQTT packet received from TCP or SSL connection */
        IOT_MQTT_Yield(pclient, 200);

        HAL_SleepMs(2000);
		HAL_SleepMs(2000);
		HAL_SleepMs(2000);
		HAL_SleepMs(2000);
		HAL_SleepMs(2000);
		HAL_SleepMs(2000);
		HAL_SleepMs(2000);

    }
   // while (cnt < 1);
    while (cnt < 700);

	printf("****************\n");
	
    IOT_MQTT_Unsubscribe(pclient, TOPIC_DATA);

    HAL_SleepMs(200);

//    IOT_MQTT_Destroy(&pclient);

do_exit:
/*    if (NULL != msg_buf) {
        HAL_Free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        HAL_Free(msg_readbuf);
    } */

 
	pthread_exit(NULL);
	
}


int pclient_init(void)
{
	int rc = 0;
    iotx_conn_info_pt pconn_info;
	iotx_mqtt_param_t mqtt_params;
	
	if (NULL == (msg_buf = (char *)HAL_Malloc(OTA_MQTT_MSGLEN))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    if (NULL == (msg_readbuf = (char *)HAL_Malloc(OTA_MQTT_MSGLEN))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }
	
	/* Device AUTH */
    if (0 != IOT_SetupConnInfo(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, (void **)&pconn_info)) {
        EXAMPLE_TRACE("AUTH request failed!");
        rc = -1;
        goto do_exit;
    }
	
	/* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port = pconn_info->port;
    mqtt_params.host = pconn_info->host_name;
    mqtt_params.client_id = pconn_info->client_id;
    mqtt_params.username = pconn_info->username;
    mqtt_params.password = pconn_info->password;
    mqtt_params.pub_key = pconn_info->pub_key;

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = OTA_MQTT_MSGLEN;
    mqtt_params.pwrite_buf = msg_buf;
    mqtt_params.write_buf_size = OTA_MQTT_MSGLEN;

    mqtt_params.handle_event.h_fp = event_handle;
    mqtt_params.handle_event.pcontext = NULL;

	
	/* Construct a MQTT client with specify parameter */
    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        rc = -1;
        goto do_exit;
    }

	return 0;
do_exit:
	
	if (NULL != pclient) {
		IOT_MQTT_Destroy(&pclient);
	}

	if (NULL != msg_buf) {
        HAL_Free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        HAL_Free(msg_readbuf);
    }
	
	return rc;
}


void *ota_thread(void* arg)
{
	#define OTA_BUF_LEN        (5000)

    int rc = 0, ota_over = 0;
    void *h_ota = NULL;
    FILE *fp;
    char buf_ota[OTA_BUF_LEN];

    if (NULL == (fp = fopen("ota.bin", "wb+"))) {
        EXAMPLE_TRACE("open file failed");
        goto do_exit;
    }

    h_ota = IOT_OTA_Init(PRODUCT_KEY, DEVICE_NAME, pclient);
    if (NULL == h_ota) {
        rc = -1;
        EXAMPLE_TRACE("initialize OTA failed");
        goto do_exit;
    }

    if (0 != IOT_OTA_ReportVersion(h_ota, "iotx_ver_1.0.0")) {
        rc = -1;
        EXAMPLE_TRACE("report OTA version failed");
        goto do_exit;
    }

    HAL_SleepMs(1000);

    do {
        uint32_t firmware_valid;

        EXAMPLE_TRACE("wait ota upgrade command....");

        /* handle the MQTT packet received from TCP or SSL connection */
        IOT_MQTT_Yield(pclient, 200);

        if (IOT_OTA_IsFetching(h_ota)) {
            uint32_t last_percent = 0, percent = 0;
            char version[128], md5sum[33];
            uint32_t len, size_downloaded, size_file;
            do {

                len = IOT_OTA_FetchYield(h_ota, buf_ota, OTA_BUF_LEN, 1);
                if (len > 0) {
                    if (1 != fwrite(buf_ota, len, 1, fp)) {
                        EXAMPLE_TRACE("write data to file failed");
                        rc = -1;
                        break;
                    }
                } else {
                    IOT_OTA_ReportProgress(h_ota, IOT_OTAP_FETCH_FAILED, NULL);
                    EXAMPLE_TRACE("ota fetch fail");
                }

                /* get OTA information */
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_FETCHED_SIZE, &size_downloaded, 4);
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_FILE_SIZE, &size_file, 4);
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_MD5SUM, md5sum, 33);
		printf("*******\nmd5 = %s\n" , md5sum);
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_VERSION, version, 128);

                last_percent = percent;
                percent = (size_downloaded * 100) / size_file;
                if (percent - last_percent > 0) {
                    IOT_OTA_ReportProgress(h_ota, (IOT_OTA_Progress_t)percent, NULL);
                    IOT_OTA_ReportProgress(h_ota, (IOT_OTA_Progress_t)percent, "hello");
                }
                IOT_MQTT_Yield(pclient, 100);
            } while (!IOT_OTA_IsFetchFinish(h_ota));

            IOT_OTA_Ioctl(h_ota, IOT_OTAG_CHECK_FIRMWARE, &firmware_valid, 4);
            if (0 == firmware_valid) {
                EXAMPLE_TRACE("The firmware is invalid");
            } else {
                EXAMPLE_TRACE("The firmware is valid");
            }

            ota_over = 1;
        }
        HAL_SleepMs(2000);
    } while (!ota_over);

    HAL_SleepMs(200);



do_exit:

    if (NULL != h_ota) {
        IOT_OTA_Deinit(h_ota);
    }

    if (NULL != fp) {
        fclose(fp);
    }
    pthread_exit(NULL);
}



int mqtt_client(void)
{
#define OTA_BUF_LEN        (5000)

    int rc = 0, ota_over = 0;
    void *pclient = NULL, *h_ota = NULL;
    iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;
    char *msg_buf = NULL, *msg_readbuf = NULL;
    FILE *fp;
    char buf_ota[OTA_BUF_LEN];

    if (NULL == (fp = fopen("ota.bin", "wb+"))) {
        EXAMPLE_TRACE("open file failed");
        goto do_exit;
    }


    if (NULL == (msg_buf = (char *)HAL_Malloc(OTA_MQTT_MSGLEN))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    if (NULL == (msg_readbuf = (char *)HAL_Malloc(OTA_MQTT_MSGLEN))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    /* Device AUTH */
    if (0 != IOT_SetupConnInfo(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, (void **)&pconn_info)) {
        EXAMPLE_TRACE("AUTH request failed!");
        rc = -1;
        goto do_exit;
    }

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port = pconn_info->port;
    mqtt_params.host = pconn_info->host_name;
    mqtt_params.client_id = pconn_info->client_id;
    mqtt_params.username = pconn_info->username;
    mqtt_params.password = pconn_info->password;
    mqtt_params.pub_key = pconn_info->pub_key;

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = OTA_MQTT_MSGLEN;
    mqtt_params.pwrite_buf = msg_buf;
    mqtt_params.write_buf_size = OTA_MQTT_MSGLEN;

    mqtt_params.handle_event.h_fp = event_handle;
    mqtt_params.handle_event.pcontext = NULL;


    /* Construct a MQTT client with specify parameter */
    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        rc = -1;
        goto do_exit;
    }
    h_ota = IOT_OTA_Init(PRODUCT_KEY, DEVICE_NAME, pclient);
    if (NULL == h_ota) {
        rc = -1;
        EXAMPLE_TRACE("initialize OTA failed");
        goto do_exit;
    }

    if (0 != IOT_OTA_ReportVersion(h_ota, "iotx_ver_1.0.0")) {
        rc = -1;
        EXAMPLE_TRACE("report OTA version failed");
        goto do_exit;
    }

    HAL_SleepMs(1000);

    do {
        uint32_t firmware_valid;

        EXAMPLE_TRACE("wait ota upgrade command....");

        /* handle the MQTT packet received from TCP or SSL connection */
        IOT_MQTT_Yield(pclient, 200);

        if (IOT_OTA_IsFetching(h_ota)) {
            uint32_t last_percent = 0, percent = 0;
            char version[128], md5sum[33];
            uint32_t len, size_downloaded, size_file;
            do {

                len = IOT_OTA_FetchYield(h_ota, buf_ota, OTA_BUF_LEN, 1);
                if (len > 0) {
                    if (1 != fwrite(buf_ota, len, 1, fp)) {
                        EXAMPLE_TRACE("write data to file failed");
                        rc = -1;
                        break;
                    }
                } else {
                    IOT_OTA_ReportProgress(h_ota, IOT_OTAP_FETCH_FAILED, NULL);
                    EXAMPLE_TRACE("ota fetch fail");
                }

                /* get OTA information */
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_FETCHED_SIZE, &size_downloaded, 4);
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_FILE_SIZE, &size_file, 4);
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_MD5SUM, md5sum, 33);
		printf("*******\nmd5 = %s\n" , md5sum);
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_VERSION, version, 128);

                last_percent = percent;
                percent = (size_downloaded * 100) / size_file;
                if (percent - last_percent > 0) {
                    IOT_OTA_ReportProgress(h_ota, (IOT_OTA_Progress_t)percent, NULL);
                    IOT_OTA_ReportProgress(h_ota, (IOT_OTA_Progress_t)percent, "hello");
                }
                IOT_MQTT_Yield(pclient, 100);
            } while (!IOT_OTA_IsFetchFinish(h_ota));

            IOT_OTA_Ioctl(h_ota, IOT_OTAG_CHECK_FIRMWARE, &firmware_valid, 4);
            if (0 == firmware_valid) {
                EXAMPLE_TRACE("The firmware is invalid");
            } else {
                EXAMPLE_TRACE("The firmware is valid");
            }

            ota_over = 1;
        }
        HAL_SleepMs(2000);
    } while (!ota_over);

    HAL_SleepMs(200);



do_exit:

    if (NULL != h_ota) {
        IOT_OTA_Deinit(h_ota);
    }

    if (NULL != pclient) {
        IOT_MQTT_Destroy(&pclient);
    }

    if (NULL != msg_buf) {
        HAL_Free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        HAL_Free(msg_readbuf);
    }

    if (NULL != fp) {
        fclose(fp);
    }

    return rc;
}





void event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            EXAMPLE_TRACE("undefined event occur.");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT:
            EXAMPLE_TRACE("MQTT disconnect.");
            break;

        case IOTX_MQTT_EVENT_RECONNECT:
            EXAMPLE_TRACE("MQTT reconnect.");
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("subscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
            EXAMPLE_TRACE("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
            EXAMPLE_TRACE("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            EXAMPLE_TRACE("publish success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
            EXAMPLE_TRACE("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            EXAMPLE_TRACE("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECVEIVED:
            EXAMPLE_TRACE("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
                          topic_info->topic_len,
                          topic_info->ptopic,
                          topic_info->payload_len,
                          topic_info->payload);
            break;

        default:
            EXAMPLE_TRACE("Should NOT arrive here.");
            break;
    }
}



static void _demo_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    /* print topic name and topic message */
    EXAMPLE_TRACE("----");
    EXAMPLE_TRACE("Topic: '%.*s' (Length: %d)",
                  ptopic_info->topic_len,
                  ptopic_info->ptopic,
                  ptopic_info->topic_len);
    EXAMPLE_TRACE("Payload: '%.*s' (Length: %d)",
                  ptopic_info->payload_len,
                  ptopic_info->payload,
                  ptopic_info->payload_len);
    EXAMPLE_TRACE("----");
}


