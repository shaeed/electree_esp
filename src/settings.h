/*

SETTINGS HEADER MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <vector>
#include <ArduinoJson.h>
#include "EmbedisWrap.h"
#include "eeprom_s.h"
#include "utils.h"

template<typename T> String getSetting(const String& key, T defaultValue);
template<typename T> String getSetting(const String& key, unsigned int index, T defaultValue);
String getSetting(const String& key);
template<typename T> bool setSetting(const String& key, T value);
template<typename T> bool setSetting(const String& key, unsigned int index, T value);
bool delSetting(const String& key);
bool delSetting(const String& key, unsigned int index);
bool hasSetting(const String& key);
bool hasSetting(const String& key, unsigned int index);
void saveSettings();
void resetSettings();
unsigned long settingsSize();
void settingsSetup();


template<typename T> String getSetting(const String& key, T defaultValue) {
    String value;
    if (!Embedis::get(key, value)) value = String(defaultValue);
    return value;
}

template<typename T> String getSetting(const String& key, unsigned int index, T defaultValue) {
    return getSetting(key + String(index), defaultValue);
}

template<typename T> bool setSetting(const String& key, T value) {
    return Embedis::set(key, String(value));
}

template<typename T> bool setSetting(const String& key, unsigned int index, T value) {
    return setSetting(key + String(index), value);
}

#endif