#ifndef OTA_H_
#define OTA_H_

void ota_pthread_create(void);

 


void *ota_thread(void* arg);

int mqtt_client(void);
void event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg);

int pclient_init(void);

void iot_pthread_create(void);
void *iot_thread(void* arg);

static void _demo_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg);





#endif



