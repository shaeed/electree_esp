/*

EEPROM HEADER MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#ifndef EEPROM_S_H
#define EEPROM_S_H

#include <EEPROM_Rotate.h>
#include "debug.h"
#include "prototypes.h"

#define EEPROM_SIZE             SPI_FLASH_SEC_SIZE  // EEPROM size in bytes (1 sector = 4096 bytes)

#define EEPROM_RELAY_STATUS     0               // Address for the relay status (1 byte)
#define EEPROM_ENERGY_COUNT     1               // Address for the energy counter (4 bytes)
#define EEPROM_CUSTOM_RESET     5               // Address for the reset reason (1 byte)
#define EEPROM_CRASH_COUNTER    6               // Address for the crash counter (1 byte)
#define EEPROM_MESSAGE_ID       7               // Address for the MQTT message id (4 bytes)
#define EEPROM_ROTATE_DATA      11              // Reserved for the EEPROM_ROTATE library (3 bytes)
#define EEPROM_DATA_END         14              // End of custom EEPROM data block

extern EEPROM_Rotate EEPROMr;

void eepromCommit();
void eepromSetup();

#endif