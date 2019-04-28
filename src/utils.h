/*

UTILS HEADER MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include "JustWifi.h"
#include "def.h"
#include "settings.h"

#include <Ticker.h>
extern Ticker _defer_reset;

extern uint8_t _reset_reason;

String getIdentifier();
void setDefaultHostname();
void setBoardName();
String getBoardName();
String getAdminPass();
String getCoreVersion();
String getCoreRevision();
unsigned char getHeartbeatMode();
unsigned char getHeartbeatInterval();
unsigned int getFreeHeap();
unsigned int getInitialFreeHeap();
unsigned int getUsedHeap();
String getEspurnaModules();
unsigned int info_bytes2sectors(size_t size);
unsigned long info_ota_space();
unsigned long info_filesystem_space();
unsigned long info_eeprom_space();
unsigned char resetReason();
void resetReason(unsigned char reason);
void reset();
void deferredReset(unsigned long delay, unsigned char reason);
bool checkNeedsReset();
void infoMemory(const char * name, unsigned int total_memory, unsigned int free_memory);
void info();
bool isNumber(const char * s);

#endif