/*

ELECTREE MAIN MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#include <Arduino.h>
#include <vector>

#include "prototypes.h"
#include "wifi.h"
#include "debug.h"
#include "eeprom_s.h"
#include "settings.h"
#include "utils.h"
#include "mqtt.h"
#include "uartmqtt.h"

std::vector<void (*)()> _loop_callbacks;
std::vector<void (*)()> _reload_callbacks;

void espurnaRegisterLoop(void (*callback)()) {
    _loop_callbacks.push_back(callback);
}

void espurnaRegisterReload(void (*callback)()) {
    _reload_callbacks.push_back(callback);
}

void espurnaReload() {
    for (unsigned char i = 0; i < _reload_callbacks.size(); i++) {
        (_reload_callbacks[i])();
    }
}

void setup() {
  getInitialFreeHeap();

  debugSetup();

  eepromSetup();

  settingsSetup();

  if (getSetting("hostname").length() == 0) {
    setDefaultHostname();
  }
  setBoardName();

  info();

  wifiSetup();

  mqttSetup();
  uartmqttSetup();

  saveSettings();

  mqttSubscribeRaw("shaeed2");
}

void loop() {
  // Call registered loop callbacks
  for (unsigned char i = 0; i < _loop_callbacks.size(); i++) {
    (_loop_callbacks[i])();
  }

  /*if(i == 0){
    i = 1;
    mqttSubscribeRaw("shaeed1");
    mqttSendRaw("shaeed", "Shaeed data/on_shaeed");
  }*/
}