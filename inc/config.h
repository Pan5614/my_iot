#ifndef CONFIG_H_
#define CONFIG_H_

#define PRODUCT_KEY             "a1CGQW6eDHz"
#define DEVICE_NAME             "third_dev"
#define DEVICE_SECRET           "BvV5cF7IeO4gTYVBofKQv0BQ56qNHqFs"



/* These are pre-defined topics */
#define TOPIC_UPDATE            "/"PRODUCT_KEY"/"DEVICE_NAME"/update"
#define TOPIC_ERROR             "/"PRODUCT_KEY"/"DEVICE_NAME"/update/error"
#define TOPIC_GET               "/"PRODUCT_KEY"/"DEVICE_NAME"/get"
#define TOPIC_DATA              "/"PRODUCT_KEY"/"DEVICE_NAME"/data"

#define OTA_MQTT_MSGLEN         (2048)



#endif
