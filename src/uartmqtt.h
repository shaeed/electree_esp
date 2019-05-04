/*

UART_MQTT HEADER MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#ifndef UARTMQTT_H
#define UARTMQTT_H

#include "mqtt.h"
#include <string.h>

#ifndef UART_MQTT_USE_SOFT
#define UART_MQTT_USE_SOFT          0           // Use SoftwareSerial
#endif

#ifndef UART_MQTT_HW_PORT
#define UART_MQTT_HW_PORT           Serial      // Hardware serial port (if UART_MQTT_USE_SOFT == 0)
#endif

#ifndef UART_MQTT_RX_PIN
#define UART_MQTT_RX_PIN            4           // RX PIN (if UART_MQTT_USE_SOFT == 1)
#endif

#ifndef UART_MQTT_TX_PIN
#define UART_MQTT_TX_PIN            5           // TX PIN (if UART_MQTT_USE_SOFT == 1)
#endif

#ifndef UART_MQTT_BAUDRATE
#define UART_MQTT_BAUDRATE          115200      // Serial speed
#endif

#ifndef UART_MQTT_TERMINATION
#define UART_MQTT_TERMINATION      '\n'         // Termination character
#endif

#define UART_MQTT_BUFFER_SIZE       200         // UART buffer size

// Internal MQTT events
#ifndef MQTT_CONNECT_EVENT
#define MQTT_CONNECT_EVENT          0
#endif

#ifndef MQTT_DISCONNECT_EVENT
#define MQTT_DISCONNECT_EVENT       1
#endif

#ifndef MQTT_MESSAGE_EVENT
#define MQTT_MESSAGE_EVENT          2
#endif

void _uartmqttReceiveUART();
void _sendOnMqtt(const char * data);
void _sendOnUart(const char * message);
void _uartmqttLoop();
void uartmqttSetup();
//char * _toCharArray(String str);
void _sendMqttStatusToBluePill();
void _sendMqttStatusToBluePill(bool status);
void _settingsGet(char * data);
//void _settingsSet(char * data);

#endif