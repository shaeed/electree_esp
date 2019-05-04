/*

UART_MQTT MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#include "uartmqtt.h"

char _uartmqttBuffer[UART_MQTT_BUFFER_SIZE];
bool _uartmqttNewData = false;

//Command idetifiers (index 0)
#define END_STRING_SYMBOL '~'
#define END_STRING_SYMBOL_S "~"
#define START_SUB_MQTT     '1' //Subscribe mqtt topic
#define START_PUB_MQTT     '2' //Publish the data (received data for bluepill)
#define START_SETT_GET     '3' //Internel setting/status get
#define START_SETT_SET     '4' //Internal setting/status set

//Settings identifiers (Index 1)
#define SETT_MQTT_STATUS        '1'
#define SETT_GET_SUB_LIST       '2' //Request blue pill to send the subscribers list


//Settings values
#define VAL_MQTT_CONNECTED     '1'
#define VAL_MQTT_DISCONNECTED  '2'


#if UART_MQTT_USE_SOFT
    #include <SoftwareSerial.h>
    SoftwareSerial _uart_mqtt_serial(UART_MQTT_RX_PIN, UART_MQTT_TX_PIN, false, UART_MQTT_BUFFER_SIZE);
    #define UART_MQTT_PORT  _uart_mqtt_serial
#else
    #define UART_MQTT_PORT  UART_MQTT_HW_PORT
#endif

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------

void _uartmqttReceiveUART() {
    static unsigned char ndx = 0;
    while (UART_MQTT_PORT.available() > 0 && _uartmqttNewData == false) {
        char rc = UART_MQTT_PORT.read();

        if (rc != UART_MQTT_TERMINATION) {
            _uartmqttBuffer[ndx] = rc;
            if (ndx < UART_MQTT_BUFFER_SIZE - 1) ndx++;
        } else {
            _uartmqttBuffer[ndx] = '\0';
            _uartmqttNewData = true;
            ndx = 0;
        }
    }
}

void _sendOnMqtt(const char * data) {
    if (_uartmqttNewData == true) {
        DEBUG_MSG_P(PSTR("[UART_MQTT] Sending on MQTT: %s\n"), data);
        mqttSend(MQTT_TOPIC_UARTIN, data);
        _uartmqttNewData = false;
    }
}

void _sendOnUart(const char * message) {
    DEBUG_MSG_P(PSTR("[UART_MQTT] Sending on UART: %s\n"), message);
    UART_MQTT_PORT.println(message);
}

int16_t getEnd(const char * data) {
    uint16_t i = 0;
    while(i < UART_MQTT_BUFFER_SIZE && data[i] != END_STRING_SYMBOL && data[i] != '\0') {
        i++;
    }

    if(i < UART_MQTT_BUFFER_SIZE && data[i] == END_STRING_SYMBOL)
        return i;
    else
        return -1;
}

void _uartProcess() {
    if (_uartmqttNewData == false)
        return;

    //Check end of data
    char * data = _uartmqttBuffer;
    int16_t end = getEnd(data);

    //Check if data processing required
    if(end > 0) {
        char * topic = NULL;
        char * msg = NULL;
        //Delete the end character
        data[end] = '\0';
        //Delete the first character
        data += 1;

        switch ( *(data-1) ) {
            case START_SUB_MQTT:
                topic = data;
                mqttSubscribe(topic);
                break;

            case START_PUB_MQTT:
                topic = strchr(data + 1, ' '); //Search the first space
                msg = topic + 1;
                //Mark the topic end and initialize topic to start of topic
                *topic = '\0';
                topic = data;
                mqttSend(topic, msg);
                //DEBUG_MSG_P(PSTR("[UART_MQTT] Send data over MQTT: %s %s\n"), topic, msg);
                break;

            case START_SETT_GET:
                _settingsGet(data);
                break;

            case START_SETT_SET:
                //_settingsSet(data);
                break;
        
            default:
                break;
        }

        //Mark data used
        _uartmqttNewData = false;
    }

}

void _settingsGet(char * data) {

    switch (data[0]) {
        case SETT_MQTT_STATUS:
            _sendMqttStatusToBluePill();
            break;
    
        default:
            break;
    }
}

void _sendMqttStatusToBluePill() {
    _sendMqttStatusToBluePill(isMqttConnected());
}

/*
 * True- Connected, False- Disconnected
 */ 
void _sendMqttStatusToBluePill(bool status) {
    char mqttStatus = status ? VAL_MQTT_CONNECTED : VAL_MQTT_DISCONNECTED;
    char * data = new char[5];
    sprintf(data, "%c%c%c%c", START_SETT_SET, SETT_MQTT_STATUS, mqttStatus, END_STRING_SYMBOL);
    _sendOnUart(data);

    delete data;
}

void _requestBluePillToSubscribe(){
    char * data = new char[5];
    sprintf(data, "%c%c%c", START_SETT_GET, SETT_GET_SUB_LIST, END_STRING_SYMBOL);
    _sendOnUart(data);

    delete data;
}

char * _toCharArray(String str) {
    int len = str.length();
    char * charr = new char[len + 1];
    str.toCharArray(charr, len+1);

    return charr;
}

void _processMqtt(const char * topic, const char * msg) {
    //Remove the esp8266 id from the topic
    String topicMag = mqttMagnitude((char *)topic);
    char * topicMagArr = _toCharArray(topicMag);

    size_t lenT = strlen(topicMagArr);
    size_t lenM = strlen(msg);
    char * data = new char[lenT + lenM + 5];
    
    //Copy data
    sprintf(data, "%c%s %s%c", START_PUB_MQTT, topicMagArr, msg, END_STRING_SYMBOL);
    _sendOnUart(data);

    delete topicMagArr;
    delete data;
}

void _uartmqttMQTTCallback(unsigned int type, const char * topic, const char * payload) {
    if (type == MQTT_CONNECT_EVENT) {
        mqttSubscribe(MQTT_TOPIC_UARTOUT);
        _sendMqttStatusToBluePill(true);
        _requestBluePillToSubscribe();
    }

    if (type == MQTT_DISCONNECT_EVENT) {
        _sendMqttStatusToBluePill(false);
    }

    if (type == MQTT_MESSAGE_EVENT) {

        // Match topic
        String t = mqttMagnitude((char *) topic);
        if (t.equals(MQTT_TOPIC_UARTOUT)) {
            _sendOnUart(payload);
        } else {
            _processMqtt(topic, payload);
        }
    }
}

// -----------------------------------------------------------------------------
// SETUP & LOOP
// -----------------------------------------------------------------------------

void _uartmqttLoop() {
    _uartmqttReceiveUART();
    _uartProcess();
    _sendOnMqtt(_uartmqttBuffer);
}

void uartmqttSetup() {
    // Init port
	// Commented as using the default serial which will be enable by main.cpp
    //UART_MQTT_PORT.begin(UART_MQTT_BAUDRATE);

    // Register MQTT callbackj
    mqttRegister(_uartmqttMQTTCallback);

    // Register loop
    espurnaRegisterLoop(_uartmqttLoop);
}
