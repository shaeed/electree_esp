/*

MQTT MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#include "mqtt.h"

AsyncMqttClient _mqtt;

bool _mqtt_enabled = true;
bool _mqtt_use_json = false;
unsigned long _mqtt_reconnect_delay = MQTT_RECONNECT_DELAY_MIN;
unsigned long _mqtt_last_connection = 0;
bool _mqtt_connecting = false;
unsigned char _mqtt_qos = MQTT_QOS;
bool _mqtt_retain = MQTT_RETAIN;
unsigned long _mqtt_keepalive = MQTT_KEEPALIVE;
String _mqtt_topic_base;
String _mqtt_topic_json;
String _mqtt_setter;
String _mqtt_getter;
bool _mqtt_forward;
char *_mqtt_user = 0;
char *_mqtt_pass = 0;
char *_mqtt_will;
char *_mqtt_clientid;

std::vector<mqtt_callback_f> _mqtt_callbacks;

typedef struct {
    unsigned char parent = 255;
    char * topic;
    char * message = NULL;
} mqtt_message_t;
std::vector<mqtt_message_t> _mqtt_queue;
Ticker _mqtt_flush_ticker;

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------

void _mqttConnect() {
    // Do not connect if already connected or still trying to connect
    if (_mqtt.connected() || _mqtt_connecting) return;

    // Check reconnect interval
    if (millis() - _mqtt_last_connection < _mqtt_reconnect_delay) return;

    // Increase the reconnect delay
    _mqtt_reconnect_delay += MQTT_RECONNECT_DELAY_STEP;
    if (_mqtt_reconnect_delay > MQTT_RECONNECT_DELAY_MAX) {
        _mqtt_reconnect_delay = MQTT_RECONNECT_DELAY_MAX;
    }

    String h = getSetting("mqttServer", MQTT_SERVER);
    char * host = strdup(h.c_str());

    unsigned int port = getSetting("mqttPort", MQTT_PORT).toInt();

    if (_mqtt_user) free(_mqtt_user);
    if (_mqtt_pass) free(_mqtt_pass);
    if (_mqtt_will) free(_mqtt_will);
    if (_mqtt_clientid) free(_mqtt_clientid);

    String user = getSetting("mqttUser", MQTT_USER);
    _mqttPlaceholders(&user);
    _mqtt_user = strdup(user.c_str());
    _mqtt_pass = strdup(getSetting("mqttPassword", MQTT_PASS).c_str());
    _mqtt_will = strdup(mqttTopic(MQTT_TOPIC_STATUS, false).c_str());
    String clientid = getSetting("mqttClientID", getIdentifier());
    _mqttPlaceholders(&clientid);
    _mqtt_clientid = strdup(clientid.c_str());

    DEBUG_MSG_P(PSTR("[MQTT] Connecting to broker at %s:%d\n"), host, port);

    _mqtt_connecting = true;
   
    _mqtt.setServer(host, port);
    _mqtt.setClientId(_mqtt_clientid);
    _mqtt.setKeepAlive(_mqtt_keepalive);
    _mqtt.setCleanSession(false);
    _mqtt.setWill(_mqtt_will, _mqtt_qos, _mqtt_retain, "0");
    if ((strlen(_mqtt_user) > 0) && (strlen(_mqtt_pass) > 0)) {
        DEBUG_MSG_P(PSTR("[MQTT] Connecting as user %s\n"), _mqtt_user);
        _mqtt.setCredentials(_mqtt_user, _mqtt_pass);
    }
   
    #if ASYNC_TCP_SSL_ENABLED
   
        bool secure = getSetting("mqttUseSSL", MQTT_SSL_ENABLED).toInt() == 1;
        _mqtt.setSecure(secure);
        if (secure) {
            DEBUG_MSG_P(PSTR("[MQTT] Using SSL\n"));
            unsigned char fp[20] = {0};
            if (sslFingerPrintArray(getSetting("mqttFP", MQTT_SSL_FINGERPRINT).c_str(), fp)) {
                _mqtt.addServerFingerprint(fp);
            } else {
                DEBUG_MSG_P(PSTR("[MQTT] Wrong fingerprint\n"));
            }
        }
   
    #endif // ASYNC_TCP_SSL_ENABLED
   
    DEBUG_MSG_P(PSTR("[MQTT] Client ID: %s\n"), _mqtt_clientid);
    DEBUG_MSG_P(PSTR("[MQTT] QoS: %d\n"), _mqtt_qos);
    DEBUG_MSG_P(PSTR("[MQTT] Retain flag: %d\n"), _mqtt_retain ? 1 : 0);
    DEBUG_MSG_P(PSTR("[MQTT] Keepalive time: %ds\n"), _mqtt_keepalive);
    DEBUG_MSG_P(PSTR("[MQTT] Will topic: %s\n"), _mqtt_will);
   
    _mqtt.connect();
    free(host);
}

void _mqttPlaceholders(String *text) {
    text->replace("{hostname}", getSetting("hostname"));
    text->replace("{magnitude}", "#");
    
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    text->replace("{mac}", mac);
}

void _mqttConfigure() {
    // Get base topic
    _mqtt_topic_base = getSetting("mqttTopic", MQTT_TOPIC);
    if (_mqtt_topic_base.endsWith("/")) _mqtt_topic_base.remove(_mqtt_topic_base.length()-1);

    // Placeholders
    _mqttPlaceholders(&_mqtt_topic_base);
    if (_mqtt_topic_base.indexOf("#") == -1) _mqtt_topic_base = _mqtt_topic_base + "/#";

    // Getters and setters
    _mqtt_setter = getSetting("mqttSetter", MQTT_SETTER);
    _mqtt_getter = getSetting("mqttGetter", MQTT_GETTER);
    _mqtt_forward = !_mqtt_getter.equals(_mqtt_setter);

    // MQTT options
    _mqtt_qos = getSetting("mqttQoS", MQTT_QOS).toInt();
    _mqtt_retain = getSetting("mqttRetain", MQTT_RETAIN).toInt() == 1;
    _mqtt_keepalive = getSetting("mqttKeep", MQTT_KEEPALIVE).toInt();
    if (getSetting("mqttClientID").length() == 0) delSetting("mqttClientID");

    // Enable
    if (getSetting("mqttServer", MQTT_SERVER).length() == 0) {
        mqttEnabled(false);
    } else {
        //_mqtt_enabled = getSetting("mqttEnabled", true).toInt() == 1;
        mqttEnabled(getSetting("mqttEnabled", true).toInt() == 1);
    }
    _mqtt_use_json = (getSetting("mqttUseJson", MQTT_USE_JSON).toInt() == 1);
    mqttQueueTopic(MQTT_TOPIC_JSON);

    _mqtt_reconnect_delay = MQTT_RECONNECT_DELAY_MIN;
}

unsigned long _mqttNextMessageId() {
    static unsigned long id = 0;

    // just reboot, get last count from EEPROM
    if (id == 0) {

        // read id from EEPROM and shift it
        id = EEPROMr.read(EEPROM_MESSAGE_ID);
        if (id == 0xFF) {
            // There was nothing in EEPROM,
            // next message is first message
            id = 0;
        } else {
            id = (id << 8) + EEPROMr.read(EEPROM_MESSAGE_ID + 1);
            id = (id << 8) + EEPROMr.read(EEPROM_MESSAGE_ID + 2);
            id = (id << 8) + EEPROMr.read(EEPROM_MESSAGE_ID + 3);

            // Calculate next block and start from there
            id = MQTT_MESSAGE_ID_SHIFT * (1 + (id / MQTT_MESSAGE_ID_SHIFT));
        }
    }

    // Save to EEPROM every MQTT_MESSAGE_ID_SHIFT
    if (id % MQTT_MESSAGE_ID_SHIFT == 0) {
        EEPROMr.write(EEPROM_MESSAGE_ID + 0, (id >> 24) & 0xFF);
        EEPROMr.write(EEPROM_MESSAGE_ID + 1, (id >> 16) & 0xFF);
        EEPROMr.write(EEPROM_MESSAGE_ID + 2, (id >>  8) & 0xFF);
        EEPROMr.write(EEPROM_MESSAGE_ID + 3, (id >>  0) & 0xFF);
        eepromCommit();
    }

    id++;
    return id;
}

// -----------------------------------------------------------------------------
// MQTT Callbacks
// -----------------------------------------------------------------------------

void _mqttCallback(unsigned int type, const char * topic, const char * payload) {
    if (type == MQTT_CONNECT_EVENT) {
        // Subscribe to internal action topics
        mqttSubscribe(MQTT_TOPIC_ACTION);
    }

    if (type == MQTT_MESSAGE_EVENT) {
        // Match topic
        String t = mqttMagnitude((char *) topic);

        // Actions
        if (t.equals(MQTT_TOPIC_ACTION)) {
            if (strcmp(payload, MQTT_ACTION_RESET) == 0) {
                deferredReset(100, CUSTOM_RESET_MQTT);
            }
        }
    }
}

void _mqttOnConnect() {
    DEBUG_MSG_P(PSTR("[MQTT] Connected!\n"));
    _mqtt_reconnect_delay = MQTT_RECONNECT_DELAY_MIN;
    _mqtt_last_connection = millis();

    // Clean subscriptions
    mqttUnsubscribeRaw("#");

    // Send connect event to subscribers
	//"Shaeed"- To subscribe again
    for (unsigned char i = 0; i < _mqtt_callbacks.size(); i++) {
        (_mqtt_callbacks[i])(MQTT_CONNECT_EVENT, NULL, NULL);
    }
}

void _mqttOnDisconnect() {
    // Reset reconnection delay
    _mqtt_last_connection = millis();
    _mqtt_connecting = false;

    DEBUG_MSG_P(PSTR("[MQTT] Disconnected!\n"));

    // Send disconnect event to subscribers
    for (unsigned char i = 0; i < _mqtt_callbacks.size(); i++) {
        (_mqtt_callbacks[i])(MQTT_DISCONNECT_EVENT, NULL, NULL);
    }
}

void _mqttOnMessage(char* topic, char* payload, unsigned int len) {
    if (len == 0) return;

    char message[len + 1];
    strlcpy(message, (char *) payload, len + 1);

    #if MQTT_SKIP_RETAINED
        if (millis() - _mqtt_last_connection < MQTT_SKIP_TIME) {
            DEBUG_MSG_P(PSTR("[MQTT] Received %s => %s - SKIPPED\n"), topic, message);
			return;
		}
    #endif
    DEBUG_MSG_P(PSTR("[MQTT] Received %s => %s\n"), topic, message);

    // Send message event to subscribers
    for (unsigned char i = 0; i < _mqtt_callbacks.size(); i++) {
        (_mqtt_callbacks[i])(MQTT_MESSAGE_EVENT, topic, message);
    }
}

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

bool isMqttConnected(){
    return _mqtt.connected();
}

/**
    Returns the magnitude part of a topic

    @param topic the full MQTT topic
    @return String object with the magnitude part.
*/
String mqttMagnitude(char * topic) {
    String pattern = _mqtt_topic_base + _mqtt_setter;
    int position = pattern.indexOf("#");
    if (position == -1) return String();
    String start = pattern.substring(0, position);
    String end = pattern.substring(position + 1);

    String magnitude = String(topic);
    if (magnitude.startsWith(start) && magnitude.endsWith(end)) {
        magnitude.replace(start, "");
        magnitude.replace(end, "");
    } else {
        magnitude = String();
    }

    return magnitude;
}

/**
    Returns a full MQTT topic from the magnitude

    @param magnitude the magnitude part of the topic.
    @param is_set whether to build a command topic (true)
        or a state topic (false).
    @return String full MQTT topic.
*/
String mqttTopic(const char * magnitude, bool is_set) {
    String output = _mqtt_topic_base;
    output.replace("#", magnitude);
    output += is_set ? _mqtt_setter : _mqtt_getter;
    return output;
}

String mqttTopic(const char * magnitude) {
    return mqttTopic(magnitude, false);
}

/**
    Returns a full MQTT topic from the magnitude

    @param magnitude the magnitude part of the topic.
    @param index index of the magnitude when more than one such magnitudes.
    @param is_set whether to build a command topic (true)
        or a state topic (false).
    @return String full MQTT topic.
*/
String mqttTopic(const char * magnitude, unsigned int index, bool is_set) {
    char buffer[strlen(magnitude)+5];
    snprintf_P(buffer, sizeof(buffer), PSTR("%s/%d"), magnitude, index);
    return mqttTopic(buffer, is_set);
}

// -----------------------------------------------------------------------------

void mqttSendRaw(const char * topic, const char * message, bool retain) {
    if (_mqtt.connected()) {
        unsigned int packetId = _mqtt.publish(topic, _mqtt_qos, retain, message);
        //DEBUG_MSG_P(PSTR("[MQTT] Sending %s => %s (PID %d)\n"), topic, message, packetId);
    }
}

void mqttSendRaw(const char * topic, const char * message) {
    mqttSendRaw (topic, message, _mqtt_retain);
}

void mqttSend(const char * topic, const char * message, bool force, bool retain) {
    bool useJson = force ? false : _mqtt_use_json;

    // Equeue message
    if (useJson) {

        // Set default queue topic
        mqttQueueTopic(MQTT_TOPIC_JSON);

        // Enqueue new message
        mqttEnqueue(topic, message);

        // Reset flush timer
        _mqtt_flush_ticker.once_ms(MQTT_USE_JSON_DELAY, mqttFlush);

    // Send it right away
    } else {
        mqttSendRaw(mqttTopic(topic, false).c_str(), message, retain);
    }
}

void mqttSend(const char * topic, const char * message, bool force) {
    mqttSend(topic, message, force, _mqtt_retain);
}

void mqttSend(const char * topic, const char * message) {
    mqttSend(topic, message, false);
}

void mqttSend(const char * topic, unsigned int index, const char * message, bool force, bool retain) {
    char buffer[strlen(topic)+5];
    snprintf_P(buffer, sizeof(buffer), PSTR("%s/%d"), topic, index);
    mqttSend(buffer, message, force, retain);
}

void mqttSend(const char * topic, unsigned int index, const char * message, bool force) {
    mqttSend(topic, index, message, force, _mqtt_retain);
}

void mqttSend(const char * topic, unsigned int index, const char * message) {
    mqttSend(topic, index, message, false);
}

// -----------------------------------------------------------------------------

unsigned char _mqttBuildTree(JsonObject& root, char parent) {
    unsigned char count = 0;

    // Add enqueued messages
    for (unsigned char i=0; i<_mqtt_queue.size(); i++) {
        mqtt_message_t element = _mqtt_queue[i];
        if (element.parent == parent) {
            ++count;
            JsonObject& elements = root.createNestedObject(element.topic);
            unsigned char num = _mqttBuildTree(elements, i);
            if (0 == num) {
                if (isNumber(element.message)) {
                    double value = atof(element.message);
                    if (value == int(value)) {
                        root.set(element.topic, int(value));
                    } else {
                        root.set(element.topic, value);
                    }
                } else {
                    root.set(element.topic, element.message);
                }
            }
        }
    }

    return count;
}

void mqttFlush() {
    if (!_mqtt.connected()) return;
    if (_mqtt_queue.size() == 0) return;

    // Build tree recursively
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    _mqttBuildTree(root, 255);

    #if MQTT_ENQUEUE_MAC
        root[MQTT_TOPIC_MAC] = WiFi.macAddress();
    #endif
    #if MQTT_ENQUEUE_HOSTNAME
        root[MQTT_TOPIC_HOSTNAME] = getSetting("hostname");
    #endif
    #if MQTT_ENQUEUE_IP
        root[MQTT_TOPIC_IP] = getIP();
    #endif
    #if MQTT_ENQUEUE_MESSAGE_ID
        root[MQTT_TOPIC_MESSAGE_ID] = _mqttNextMessageId();
    #endif

    // Send
    String output;
    root.printTo(output);
    jsonBuffer.clear();

    mqttSendRaw(_mqtt_topic_json.c_str(), output.c_str(), false);

    // Clear queue
    for (unsigned char i = 0; i < _mqtt_queue.size(); i++) {
        mqtt_message_t element = _mqtt_queue[i];
        free(element.topic);
        if (element.message) {
            free(element.message);
        }
    }
    _mqtt_queue.clear();
}

void mqttQueueTopic(const char * topic) {
    String t = mqttTopic(topic, false);
    if (!t.equals(_mqtt_topic_json)) {
        mqttFlush();
        _mqtt_topic_json = t;
    }
}

int8_t mqttEnqueue(const char * topic, const char * message, unsigned char parent) {

    // Queue is not meant to send message "offline"
    // We must prevent the queue does not get full while offline
    if (!_mqtt.connected()) return -1;

    // Force flusing the queue if the MQTT_QUEUE_MAX_SIZE has been reached
    if (_mqtt_queue.size() >= MQTT_QUEUE_MAX_SIZE) mqttFlush();

    int8_t index = _mqtt_queue.size();

    // Enqueue new message
    mqtt_message_t element;
    element.parent = parent;
    element.topic = strdup(topic);
    if (NULL != message) {
        element.message = strdup(message);
    }
    _mqtt_queue.push_back(element);

    return index;
}

int8_t mqttEnqueue(const char * topic, const char * message) {
    return mqttEnqueue(topic, message, 255);
}

// -----------------------------------------------------------------------------

void mqttSubscribeRaw(const char * topic) {
    if (_mqtt.connected() && (strlen(topic) > 0)) {
        unsigned int packetId = _mqtt.subscribe(topic, _mqtt_qos);
        DEBUG_MSG_P(PSTR("[MQTT] Subscribing to %s (PID %d)\n"), topic, packetId);
    }
}

void mqttSubscribe(const char * topic) {
    mqttSubscribeRaw(mqttTopic(topic, true).c_str());
}

void mqttUnsubscribeRaw(const char * topic) {
    if (_mqtt.connected() && (strlen(topic) > 0)) {
        unsigned int packetId = _mqtt.unsubscribe(topic);
        DEBUG_MSG_P(PSTR("[MQTT] Unsubscribing to %s (PID %d)\n"), topic, packetId);
    }
}

void mqttUnsubscribe(const char * topic) {
    mqttUnsubscribeRaw(mqttTopic(topic, true).c_str());
}

// -----------------------------------------------------------------------------

void mqttEnabled(bool status) {
    _mqtt_enabled = status;
}

bool mqttEnabled() {
    return _mqtt_enabled;
}

bool mqttConnected() {
    return _mqtt.connected();
}

void mqttDisconnect() {
    if (_mqtt.connected()) {
        DEBUG_MSG_P(PSTR("[MQTT] Disconnecting\n"));
        _mqtt.disconnect();
    }
}

bool mqttForward() {
    return _mqtt_forward;
}

void mqttRegister(mqtt_callback_f callback) {
    _mqtt_callbacks.push_back(callback);
}

void mqttSetBroker(IPAddress ip, unsigned int port) {
    setSetting("mqttServer", ip.toString());
    setSetting("mqttPort", port);
    mqttEnabled(MQTT_AUTOCONNECT);
}

void mqttSetBrokerIfNone(IPAddress ip, unsigned int port) {
    if (getSetting("mqttServer", MQTT_SERVER).length() == 0) mqttSetBroker(ip, port);
}

void mqttReset() {
    _mqttConfigure();
    mqttDisconnect();
}

// -----------------------------------------------------------------------------
// Initialization
// -----------------------------------------------------------------------------

void mqttSetup() {
     _mqtt.onConnect([](bool sessionPresent) {
         _mqttOnConnect();
     });
     _mqtt.onDisconnect([](AsyncMqttClientDisconnectReason reason) {
         if (reason == AsyncMqttClientDisconnectReason::TCP_DISCONNECTED) {
             DEBUG_MSG_P(PSTR("[MQTT] TCP Disconnected\n"));
         }
         if (reason == AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED) {
             DEBUG_MSG_P(PSTR("[MQTT] Identifier Rejected\n"));
         }
         if (reason == AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE) {
             DEBUG_MSG_P(PSTR("[MQTT] Server unavailable\n"));
         }
         if (reason == AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS) {
             DEBUG_MSG_P(PSTR("[MQTT] Malformed credentials\n"));
         }
         if (reason == AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED) {
             DEBUG_MSG_P(PSTR("[MQTT] Not authorized\n"));
         }
         #if ASYNC_TCP_SSL_ENABLED
         if (reason == AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT) {
             DEBUG_MSG_P(PSTR("[MQTT] Bad fingerprint\n"));
         }
         #endif
         _mqttOnDisconnect();
     });
     _mqtt.onMessage([](char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
		 _mqttOnMessage(topic, payload, len);
		 
     });
     _mqtt.onSubscribe([](uint16_t packetId, uint8_t qos) {
         DEBUG_MSG_P(PSTR("[MQTT] Subscribe ACK for PID %d\n"), packetId);
     });
     _mqtt.onPublish([](uint16_t packetId) {
         DEBUG_MSG_P(PSTR("[MQTT] Publish ACK for PID %d\n"), packetId);
     });

    _mqttConfigure();
    mqttRegister(_mqttCallback);

    // Main callbacks
    espurnaRegisterLoop(mqttLoop);
    espurnaRegisterReload(_mqttConfigure);
}

void mqttLoop() {
    if (WiFi.status() != WL_CONNECTED) return;
	_mqttConnect();
}
