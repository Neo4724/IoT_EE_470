/*
 * ---------------------------------------------------------------------------
 * Project/Program Name : EE 470 ESP8266 IoT Sensor System
 * File Name            : main.cpp
 * Author               : Antonio Kassis
 * Date                 : 11/10/2026
 * Version              : 6.0
 *
 * Purpose:
 * Read photoresistor voltage and HC-SR04 distance when the pushbutton
 * or tilt switch is activated. Obtain the current timestamp and send
 * the measurement to the remote database. If delivery fails, retry
 * the exact same measurement every 30 seconds until confirmed.
 *
 * Inputs:
 * - Photoresistor on A0
 * - HC-SR04 ultrasonic sensor
 * - Pushbutton
 * - Tilt switch
 * - Serial time-zone selection
 *
 * Outputs:
 * - HTTP GET request
 * - Sensor data
 * - Server response
 * - RSSI
 *
 * Dependencies:
 * - ESP8266WiFi
 * - ESP8266HTTPClient
 * - ArduinoJson
 * ---------------------------------------------------------------------------
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>


// --------------------------------------------------
// WIFI
// --------------------------------------------------

const char* WIFI_SSID =
    "Samsung Galaxy A10e 8801";

const char* WIFI_PASSWORD =
    "0pp0pp0pp";


// --------------------------------------------------
// SERVER
// --------------------------------------------------

const char* SERVER_URL =
    "https://antoniokassis.com/esp_insert.php";


// --------------------------------------------------
// PINS
// --------------------------------------------------

#define LIGHT_PIN A0

#define TRIG_PIN D1
#define ECHO_PIN D2

#define BUTTON_PIN D5
#define TILT_PIN D6


// --------------------------------------------------
// TIME ZONE
// --------------------------------------------------

String selectedTimeZone =
    "America/Los_Angeles";


// --------------------------------------------------
// SWITCH STATE
// --------------------------------------------------

bool previousButtonState = false;
bool previousTiltState = false;

unsigned long lastButtonEvent = 0;
unsigned long lastTiltEvent = 0;

const unsigned long DEBOUNCE_TIME = 250;


// --------------------------------------------------
// RETRANSMISSION
// --------------------------------------------------

const unsigned long RETRY_INTERVAL = 30000;


// --------------------------------------------------
// PENDING MESSAGE
// --------------------------------------------------

struct PendingMessage {

    bool active;

    String node;
    String timestamp;

    float lightVoltage;
    float distance;

    unsigned long lastAttempt;
};


PendingMessage pending;


// --------------------------------------------------
// FUNCTION PROTOTYPES
// --------------------------------------------------

void connectWiFi();
void ensureWiFi();
void printNetworkInfo();

void printTimeZoneMenu();
void selectTimeZone();
void setTimeZone(char choice);

String getCurrentTime();

float readLightVoltage();
float readDistanceCM();

void checkSwitches();
void handleActivation(String node);

bool sendPendingMessage();

void retryPendingMessage();

String urlEncode(
    const String& text
);


// --------------------------------------------------
// SETUP
// --------------------------------------------------

void setup() {

    Serial.begin(115200);

    delay(1000);


    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    digitalWrite(
        TRIG_PIN,
        LOW
    );


    pinMode(
        BUTTON_PIN,
        INPUT_PULLUP
    );

    pinMode(
        TILT_PIN,
        INPUT_PULLUP
    );


    pending.active = false;
    pending.lastAttempt = 0;


    Serial.println();
    Serial.println(
        "EE 470 ESP8266 IoT System"
    );

    Serial.println(
        "-------------------------"
    );


    connectWiFi();

    printNetworkInfo();

    selectTimeZone();


    Serial.println();
    Serial.println(
        "System Ready"
    );
}


// --------------------------------------------------
// LOOP
// --------------------------------------------------

void loop() {

    ensureWiFi();

    checkSwitches();

    retryPendingMessage();


    if (Serial.available()) {

        char choice =
            Serial.read();


        if (
            choice >= '1' &&
            choice <= '7'
        ) {

            setTimeZone(choice);
        }
    }


    delay(20);
}


// --------------------------------------------------
// CONNECT WIFI
// --------------------------------------------------

void connectWiFi() {

    Serial.println();
    Serial.print(
        "Connecting to WiFi"
    );


    WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD
    );


    while (
        WiFi.status()
        != WL_CONNECTED
    ) {

        delay(500);

        Serial.print(".");
    }


    Serial.println();
    Serial.println(
        "WiFi connected!"
    );
}


// --------------------------------------------------
// RECONNECT WIFI IF LOST
// --------------------------------------------------

void ensureWiFi() {

    if (
        WiFi.status()
        == WL_CONNECTED
    ) {

        return;
    }


    Serial.println();
    Serial.println(
        "WiFi disconnected."
    );

    Serial.println(
        "Reconnecting..."
    );


    WiFi.disconnect();

    WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD
    );


    unsigned long start =
        millis();


    while (
        WiFi.status()
            != WL_CONNECTED
        &&
        millis() - start
            < 10000
    ) {

        delay(500);

        Serial.print(".");
    }


    Serial.println();


    if (
        WiFi.status()
        == WL_CONNECTED
    ) {

        Serial.println(
            "WiFi reconnected."
        );

    } else {

        Serial.println(
            "WiFi reconnect failed."
        );
    }
}


// --------------------------------------------------
// NETWORK INFORMATION
// --------------------------------------------------

void printNetworkInfo() {

    Serial.println();

    Serial.print(
        "IP Address: "
    );

    Serial.println(
        WiFi.localIP()
    );


    Serial.print(
        "MAC Address: "
    );

    Serial.println(
        WiFi.macAddress()
    );


    Serial.print(
        "RSSI: "
    );

    Serial.print(
        WiFi.RSSI()
    );

    Serial.println(
        " dBm"
    );
}


// --------------------------------------------------
// TIME-ZONE MENU
// --------------------------------------------------

void printTimeZoneMenu() {

    Serial.println();

    Serial.println(
        "Select Your Time Zone"
    );

    Serial.println(
        "Default = Pacific Time"
    );

    Serial.println();

    Serial.println(
        "1) Eastern"
    );

    Serial.println(
        "2) Central"
    );

    Serial.println(
        "3) Mountain"
    );

    Serial.println(
        "4) Pacific"
    );

    Serial.println(
        "5) Alaska"
    );

    Serial.println(
        "6) Hawaii"
    );

    Serial.println(
        "7) Atlantic"
    );

    Serial.println();

    Serial.println(
        "Enter 1 through 7:"
    );
}


// --------------------------------------------------
// INITIAL TIME-ZONE SELECTION
// --------------------------------------------------

void selectTimeZone() {

    printTimeZoneMenu();


    unsigned long startTime =
        millis();


    while (
        millis() - startTime
        < 10000
    ) {

        if (Serial.available()) {

            char choice =
                Serial.read();


            if (
                choice >= '1' &&
                choice <= '7'
            ) {

                setTimeZone(choice);

                return;
            }
        }
    }


    Serial.println();

    Serial.println(
        "Using default: Pacific Time"
    );


    selectedTimeZone =
        "America/Los_Angeles";
}


// --------------------------------------------------
// SET TIME ZONE
// --------------------------------------------------

void setTimeZone(char choice) {

    switch (choice) {

        case '1':

            selectedTimeZone =
                "America/New_York";

            Serial.println(
                "Selected: Eastern"
            );

            break;


        case '2':

            selectedTimeZone =
                "America/Chicago";

            Serial.println(
                "Selected: Central"
            );

            break;


        case '3':

            selectedTimeZone =
                "America/Denver";

            Serial.println(
                "Selected: Mountain"
            );

            break;


        case '4':

            selectedTimeZone =
                "America/Los_Angeles";

            Serial.println(
                "Selected: Pacific"
            );

            break;


        case '5':

            selectedTimeZone =
                "America/Anchorage";

            Serial.println(
                "Selected: Alaska"
            );

            break;


        case '6':

            selectedTimeZone =
                "Pacific/Honolulu";

            Serial.println(
                "Selected: Hawaii"
            );

            break;


        case '7':

            selectedTimeZone =
                "America/Puerto_Rico";

            Serial.println(
                "Selected: Atlantic"
            );

            break;
    }


    Serial.print(
        "Time Zone: "
    );

    Serial.println(
        selectedTimeZone
    );
}


// --------------------------------------------------
// GET CURRENT TIME
// --------------------------------------------------

String getCurrentTime() {

    if (
        WiFi.status()
        != WL_CONNECTED
    ) {

        return "";
    }


    BearSSL::WiFiClientSecure client;

    client.setInsecure();


    HTTPClient http;


    String url =
        "https://timeapi.io/api/Time/current/zone?timeZone="
        + selectedTimeZone;


    if (
        !http.begin(
            client,
            url
        )
    ) {

        return "";
    }


    int httpCode =
        http.GET();


    if (
        httpCode
        != HTTP_CODE_OK
    ) {

        http.end();

        return "";
    }


    String response =
        http.getString();


    http.end();


    JsonDocument doc;


    DeserializationError error =
        deserializeJson(
            doc,
            response
        );


    if (error) {

        return "";
    }


    String dateTime =
        doc["dateTime"] | "";


    if (
        dateTime.length()
        >= 19
    ) {

        String formatted =
            dateTime.substring(
                0,
                19
            );


        formatted.replace(
            "T",
            " "
        );


        return formatted;
    }


    return "";
}


// --------------------------------------------------
// READ LIGHT VOLTAGE
// --------------------------------------------------

float readLightVoltage() {

    int raw =
        analogRead(
            LIGHT_PIN
        );


    return
        raw *
        (3.3 / 1023.0);
}


// --------------------------------------------------
// READ DISTANCE
// --------------------------------------------------

float readDistanceCM() {

    digitalWrite(
        TRIG_PIN,
        LOW
    );

    delayMicroseconds(2);


    digitalWrite(
        TRIG_PIN,
        HIGH
    );

    delayMicroseconds(10);


    digitalWrite(
        TRIG_PIN,
        LOW
    );


    unsigned long duration =
        pulseIn(
            ECHO_PIN,
            HIGH,
            30000
        );


    if (duration == 0) {

        return -1.0;
    }


    return
        (
            duration *
            0.0343
        ) / 2.0;
}


// --------------------------------------------------
// CHECK SWITCHES
// --------------------------------------------------

void checkSwitches() {

    bool buttonActive =
        digitalRead(
            BUTTON_PIN
        ) == LOW;


    bool tiltActive =
        digitalRead(
            TILT_PIN
        ) == LOW;


    // Pushbutton
    if (
        buttonActive &&
        !previousButtonState &&
        millis() - lastButtonEvent
            > DEBOUNCE_TIME
    ) {

        lastButtonEvent =
            millis();


        handleActivation(
            "node_1"
        );
    }


    // Tilt switch
    if (
        tiltActive &&
        !previousTiltState &&
        millis() - lastTiltEvent
            > DEBOUNCE_TIME
    ) {

        lastTiltEvent =
            millis();


        handleActivation(
            "node_2"
        );
    }


    previousButtonState =
        buttonActive;

    previousTiltState =
        tiltActive;
}


// --------------------------------------------------
// NEW SENSOR ACTIVATION
// --------------------------------------------------

void handleActivation(
    String node
) {

    // Do not overwrite a message
    // that still needs delivery.
    if (pending.active) {

        Serial.println();
        Serial.println(
            "A previous message is still pending."
        );

        Serial.println(
            "New activation ignored."
        );

        return;
    }


    Serial.println();
    Serial.println(
        "NEW MEASUREMENT"
    );

    Serial.println(
        "----------------"
    );


    float lightVoltage =
        readLightVoltage();


    float distance =
        readDistanceCM();


    if (distance < 0) {

        Serial.println(
            "ERROR: HC-SR04 no echo."
        );

        return;
    }


    String timestamp =
        getCurrentTime();


    if (timestamp == "") {

        Serial.println(
            "ERROR: Could not obtain timestamp."
        );

        return;
    }


    // Save ORIGINAL values
    // before trying transmission.

    pending.active =
        true;

    pending.node =
        node;

    pending.timestamp =
        timestamp;

    pending.lightVoltage =
        lightVoltage;

    pending.distance =
        distance;

    pending.lastAttempt =
        0;


    Serial.print(
        "Node: "
    );

    Serial.println(
        pending.node
    );


    Serial.print(
        "Timestamp: "
    );

    Serial.println(
        pending.timestamp
    );


    Serial.print(
        "Light Voltage: "
    );

    Serial.print(
        pending.lightVoltage,
        3
    );

    Serial.println(
        " V"
    );


    Serial.print(
        "Distance: "
    );

    Serial.print(
        pending.distance,
        2
    );

    Serial.println(
        " cm"
    );


    Serial.print(
        "RSSI: "
    );

    Serial.print(
        WiFi.RSSI()
    );

    Serial.println(
        " dBm"
    );


    sendPendingMessage();
}


// --------------------------------------------------
// URL ENCODING
// --------------------------------------------------

String urlEncode(
    const String& text
) {

    String encoded =
        "";

    char buffer[4];


    for (
        unsigned int i = 0;
        i < text.length();
        i++
    ) {

        char c =
            text.charAt(i);


        if (
            isalnum(c) ||
            c == '-' ||
            c == '_' ||
            c == '.' ||
            c == '~'
        ) {

            encoded += c;

        } else {

            sprintf(
                buffer,
                "%%%02X",
                (unsigned char)c
            );


            encoded +=
                buffer;
        }
    }


    return encoded;
}


// --------------------------------------------------
// SEND PENDING MESSAGE
// --------------------------------------------------

bool sendPendingMessage() {

    if (!pending.active) {

        return true;
    }


    pending.lastAttempt =
        millis();


    if (
        WiFi.status()
        != WL_CONNECTED
    ) {

        Serial.println(
            "Cannot transmit: WiFi unavailable."
        );

        return false;
    }


    String url =
        String(SERVER_URL)

        + "?nodeId="
        + urlEncode(
            pending.node
        )

        + "&timeReceived="
        + urlEncode(
            pending.timestamp
        )

        + "&lightVoltage="
        + String(
            pending.lightVoltage,
            3
        )

        + "&distanceCM="
        + String(
            pending.distance,
            2
        );


    Serial.println();
    Serial.println(
        "TRANSMISSION ATTEMPT"
    );


    Serial.print(
        "Original Timestamp: "
    );

    Serial.println(
        pending.timestamp
    );


    Serial.print(
        "Original Light: "
    );

    Serial.println(
        pending.lightVoltage,
        3
    );


    Serial.print(
        "Original Distance: "
    );

    Serial.println(
        pending.distance,
        2
    );


    BearSSL::WiFiClientSecure client;

    client.setInsecure();


    HTTPClient http;


    if (
        !http.begin(
            client,
            url
        )
    ) {

        Serial.println(
            "HTTP connection failed."
        );

        return false;
    }


    int httpCode =
        http.GET();


    String response =
        http.getString();


    http.end();


    Serial.print(
        "HTTP Code: "
    );

    Serial.println(
        httpCode
    );


    Serial.print(
        "Server Response: "
    );

    Serial.println(
        response
    );


    // New message successfully stored.
    if (
        httpCode == 200 &&
        response.indexOf(
            "STORED"
        ) >= 0
    ) {

        Serial.println(
            "DELIVERY CONFIRMED"
        );


        pending.active =
            false;


        return true;
    }


    // The server already has this exact
    // node + timestamp record.
    // This also counts as successful delivery.
    if (
        httpCode == 200 &&
        response.indexOf(
            "DUPLICATE"
        ) >= 0
    ) {

        Serial.println(
            "MESSAGE ALREADY STORED"
        );

        Serial.println(
            "DELIVERY CONFIRMED"
        );


        pending.active =
            false;


        return true;
    }


    Serial.println(
        "DELIVERY NOT CONFIRMED"
    );

    Serial.println(
        "Message saved for retry."
    );


    return false;
}


// --------------------------------------------------
// AUTOMATIC RETRANSMISSION
// --------------------------------------------------

void retryPendingMessage() {

    if (!pending.active) {

        return;
    }


    if (
        millis() - pending.lastAttempt
        >= RETRY_INTERVAL
    ) {

        Serial.println();
        Serial.println(
            "30 seconds elapsed."
        );

        Serial.println(
            "RETRANSMITTING SAME MESSAGE..."
        );


        sendPendingMessage();
    }
}