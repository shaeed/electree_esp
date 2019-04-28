/*

COMMOM DEFINITIONS MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#ifndef DEF_H
#define DEF_H

#define APP_NAME                "ELECTREE"

#ifndef HOSTNAME
#define HOSTNAME                "Shaeed"
#endif


#define MANUFACTURER            "ELECTREE"
#define DEVICE                  "esp01"
#define DEVICE_NAME             MANUFACTURER "_" DEVICE     // Concatenate both to get a unique device name

#ifndef ADMIN_PASS
#define ADMIN_PASS              "Shaeed@12"     // Default password (WEB, OTA, WIFI SoftAP)
#endif

#ifndef USE_PASSWORD
#define USE_PASSWORD            1               // Insecurity caution! Disabling this will disable password querying completely.
#endif

#ifndef LOOP_DELAY_TIME
#define LOOP_DELAY_TIME         1               // Delay for this millis in the main loop [0-250] (see https://github.com/xoseperez/espurna/issues/1541)
#endif

//------------------------------------------------------------------------------
// HEARTBEAT
//------------------------------------------------------------------------------

#define HEARTBEAT_NONE              0           // Never send heartbeat
#define HEARTBEAT_ONCE              1           // Send it only once upon MQTT connection
#define HEARTBEAT_REPEAT            2           // Send it upon MQTT connection and every HEARTBEAT_INTERVAL
#define HEARTBEAT_REPEAT_STATUS     3           // Send it upon MQTT connection and every HEARTBEAT_INTERVAL only STATUS report

// Backwards compatibility check
#if defined(HEARTBEAT_ENABLED) && (HEARTBEAT_ENABLED == 0)
#define HEARTBEAT_MODE              HEARTBEAT_NONE
#endif

#ifndef HEARTBEAT_MODE
#define HEARTBEAT_MODE              HEARTBEAT_REPEAT
#endif

#ifndef HEARTBEAT_INTERVAL
#define HEARTBEAT_INTERVAL          300         // Interval between heartbeat messages (in sec)
#endif

#define UPTIME_OVERFLOW             4294967295  // Uptime overflow value

// Values that will be reported in heartbeat
#ifndef HEARTBEAT_REPORT_STATUS
#define HEARTBEAT_REPORT_STATUS     1
#endif

#ifndef HEARTBEAT_REPORT_SSID
#define HEARTBEAT_REPORT_SSID       1
#endif

#ifndef HEARTBEAT_REPORT_IP
#define HEARTBEAT_REPORT_IP         1
#endif

#ifndef HEARTBEAT_REPORT_MAC
#define HEARTBEAT_REPORT_MAC        1
#endif

#ifndef HEARTBEAT_REPORT_RSSI
#define HEARTBEAT_REPORT_RSSI       1
#endif

#ifndef HEARTBEAT_REPORT_UPTIME
#define HEARTBEAT_REPORT_UPTIME     1
#endif

#ifndef HEARTBEAT_REPORT_DATETIME
#define HEARTBEAT_REPORT_DATETIME   1
#endif

#ifndef HEARTBEAT_REPORT_FREEHEAP
#define HEARTBEAT_REPORT_FREEHEAP   1
#endif

#ifndef HEARTBEAT_REPORT_VCC
#define HEARTBEAT_REPORT_VCC        1
#endif

#ifndef HEARTBEAT_REPORT_RELAY
#define HEARTBEAT_REPORT_RELAY      1
#endif

#ifndef HEARTBEAT_REPORT_LIGHT
#define HEARTBEAT_REPORT_LIGHT      1
#endif

#ifndef HEARTBEAT_REPORT_HOSTNAME
#define HEARTBEAT_REPORT_HOSTNAME   1
#endif

#ifndef HEARTBEAT_REPORT_DESCRIPTION
#define HEARTBEAT_REPORT_DESCRIPTION 1
#endif

#ifndef HEARTBEAT_REPORT_APP
#define HEARTBEAT_REPORT_APP        1
#endif

#ifndef HEARTBEAT_REPORT_VERSION
#define HEARTBEAT_REPORT_VERSION    1
#endif

#ifndef HEARTBEAT_REPORT_BOARD
#define HEARTBEAT_REPORT_BOARD      1
#endif

#ifndef HEARTBEAT_REPORT_LOADAVG
#define HEARTBEAT_REPORT_LOADAVG    1
#endif

#ifndef HEARTBEAT_REPORT_INTERVAL
#define HEARTBEAT_REPORT_INTERVAL   0
#endif

//------------------------------------------------------------------------------
// RESET
//------------------------------------------------------------------------------

#define CUSTOM_RESET_HARDWARE       1       // Reset from hardware button
#define CUSTOM_RESET_WEB            2       // Reset from web interface
#define CUSTOM_RESET_TERMINAL       3       // Reset from terminal
#define CUSTOM_RESET_MQTT           4       // Reset via MQTT
#define CUSTOM_RESET_RPC            5       // Reset via RPC (HTTP)
#define CUSTOM_RESET_OTA            6       // Reset after successful OTA update
#define CUSTOM_RESET_HTTP           7       // Reset via HTTP GET
#define CUSTOM_RESET_NOFUSS         8       // Reset after successful NOFUSS update
#define CUSTOM_RESET_UPGRADE        9       // Reset after update from web interface
#define CUSTOM_RESET_FACTORY        10      // Factory reset from terminal
#define CUSTOM_RESET_MAX            10

#ifndef ADC_MODE_VALUE
#define ADC_MODE_VALUE                  ADC_VCC
#endif


#endif