#ifndef __MQTT_APP_H
#define __MQTT_APP_H

/** MQTT 连接状态：-1=连接中  0=断开  1=已连接 */
extern volatile int g_mqtt_connected;

void mqtt_app_start(void);

/** 上报当前的正确密码（如 "1234"） */
void mqtt_publish_password(const char *pwd);

/** 上报密码输入错误（发送 1） */
void mqtt_publish_wrong_pwd(void);

/** 上报门锁状态：1=打开，0=关闭 */
void mqtt_publish_lock_status(int open);

/** MQTT 连接后的初始化（订阅 + 初始发布），在连接建立后调用一次 */
void mqtt_app_init_session(void);

#endif
