/*

WIFI HEADER MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#ifndef WIFI_H
#define WIFI_H

#include <Ticker.h>
#include "JustWifi.h"
#include "debug.h"
#include "prototypes.h"
#include "def.h"
#include "settings.h"

#define WIFI_STATE_AP               1
#define WIFI_STATE_STA              2
#define WIFI_STATE_AP_STA           3
#define WIFI_STATE_WPS              4
#define WIFI_STATE_SMARTCONFIG      8

#define WIFI_AP_ALLWAYS             1
#define WIFI_AP_FALLBACK            2

// -----------------------------------------------------------------------------
// WIFI
// -----------------------------------------------------------------------------

#ifndef WIFI_CONNECT_TIMEOUT
#define WIFI_CONNECT_TIMEOUT        60000               // Connecting timeout for WIFI in ms
#endif

#ifndef WIFI_RECONNECT_INTERVAL
#define WIFI_RECONNECT_INTERVAL     180000              // If could not connect to WIFI, retry after this time in ms
#endif

#ifndef WIFI_MAX_NETWORKS
#define WIFI_MAX_NETWORKS           5                   // Max number of WIFI connection configurations
#endif

#ifndef WIFI_AP_CAPTIVE
#define WIFI_AP_CAPTIVE             1                   // Captive portal enabled when in AP mode
#endif

#ifndef WIFI_FALLBACK_APMODE
#define WIFI_FALLBACK_APMODE        1                   // Fallback to AP mode if no STA connection
#endif

#ifndef WIFI_SLEEP_MODE
#define WIFI_SLEEP_MODE             WIFI_NONE_SLEEP     // WIFI_NONE_SLEEP, WIFI_LIGHT_SLEEP or WIFI_MODEM_SLEEP
#endif

#ifndef WIFI_SCAN_NETWORKS
#define WIFI_SCAN_NETWORKS          1                   // Perform a network scan before connecting
#endif

// Optional hardcoded configuration (up to 2 networks)
#ifndef WIFI1_SSID
#define WIFI1_SSID                  "AKG"
#endif

#ifndef WIFI1_PASS
#define WIFI1_PASS                  "Ashwani#$45894"
#endif

#ifndef WIFI1_IP
#define WIFI1_IP                    ""
#endif

#ifndef WIFI1_GW
#define WIFI1_GW                    ""
#endif

#ifndef WIFI1_MASK
#define WIFI1_MASK                  ""
#endif

#ifndef WIFI1_DNS
#define WIFI1_DNS                   ""
#endif

#ifndef WIFI2_SSID
#define WIFI2_SSID                  ""
#endif

#ifndef WIFI2_PASS
#define WIFI2_PASS                  ""
#endif

#ifndef WIFI2_IP
#define WIFI2_IP                    ""
#endif

#ifndef WIFI2_GW
#define WIFI2_GW                    ""
#endif

#ifndef WIFI2_MASK
#define WIFI2_MASK                  ""
#endif

#ifndef WIFI2_DNS
#define WIFI2_DNS                   ""
#endif

#ifndef WIFI_RSSI_1M
#define WIFI_RSSI_1M                -30         // Calibrate it with your router reading the RSSI at 1m
#endif

#ifndef WIFI_PROPAGATION_CONST
#define WIFI_PROPAGATION_CONST      4           // This is typically something between 2.7 to 4.3 (free space is 2)
#endif


typedef std::function<void(justwifi_messages_t code, char * parameter)> wifi_callback_f;
void wifiRegister(wifi_callback_f callback);
bool wifiConnected();

void wifiSetup();
void wifiLoop();
void wifiReconnectCheck();
bool _wifiClean(unsigned char num);
void wifiDebug(WiFiMode_t modes);
void wifiRegister(wifi_callback_f callback);
String getIP();

#endif