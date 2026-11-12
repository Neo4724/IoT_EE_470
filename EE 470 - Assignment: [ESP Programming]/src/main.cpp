/*
 * ---------------------------------------------------------------------------
 * Project/Program Name : EE 470 ESP8266 IoT Sensor System
 * File Name            : main.cpp
 * Author               : Antonio Kassis
 * Date                 : 11/12/2026
 * Version              : 7.0
 *
 * Purpose:
 * Read photoresistor voltage and HC-SR04 distance when the pushbutton
 * or tilt switch is activated.
 *
 * If WiFi is available, obtain the timestamp immediately.
 *
 * If WiFi is unavailable, save the measurement time using millis().
 * When WiFi returns, obtain the current timestamp from the time server,
 * calculate how much time has elapsed since the measurement, and subtract
 * that elapsed time to reconstruct the original measurement timestamp.
 *
 * If delivery fails, retry the exact same measurement every 30 seconds
 * until confirmed.
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
 * - Reconstructed timestamp during WiFi outage
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
#include <time.h>

// --------------------------------------------------
// WIFI
// --------------------------------------------------

const char* WIFI_SSID =
    "Samsung Galaxy A10e 8801";

const char* WIFI_PASSWORD =
    "0pp0pp0pp";

// Try reconnecting every 5 seconds without
// blocking the rest of the program.
const unsigned long WIFI_RECONNECT_INTERVAL = 5000;

unsigned long lastWiFiReconnectAttempt = 0;
bool wifiWasConnected = false;


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

    // Final original timestamp that will be
    // sent to the server.
    String timestamp;

    // True once the original timestamp
    // has been determined.
    bool timestampReady;

    float lightVoltage;
    float distance;

    // millis() value when the physical
    // measurement was actually taken.
    unsigned long measurementMillis;

    // Used for 30-second retransmission.
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

String subtractSecondsFromTimestamp(
    const String& timestamp,
    unsigned long seconds
);

bool preparePendingTimestamp();

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

    digitalWrite(TRIG_PIN, LOW);


    pinMode(
        BUTTON_PIN,
        INPUT_PULLUP
    );

    pinMode(
        TILT_PIN,
        INPUT_PULLUP
    );


    pending.active = false;
    pending.timestampReady = false;
    pending.lastAttempt = 0;
    pending.measurementMillis = 0;


    Serial.println();
    Serial.println(
        "EE 470 ESP8266 IoT System"
    );

    Serial.println(
        "-------------------------"
    );


    connectWiFi();

    if (
        WiFi.status()
        == WL_CONNECTED
    ) {

        printNetworkInfo();
    }


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

    // Non-blocking WiFi reconnect.
    ensureWiFi();

    // These continue running even if
    // WiFi is unavailable.
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
// INITIAL WIFI CONNECTION
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


    unsigned long start =
        millis();


    // Only wait 10 seconds.
    // If the network is unavailable,
    // continue running offline.
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
            "WiFi connected!"
        );

        wifiWasConnected = true;

    } else {

        Serial.println(
            "WiFi unavailable."
        );

        Serial.println(
            "Starting in offline mode."
        );

        wifiWasConnected = false;
    }
}


// --------------------------------------------------
// NON-BLOCKING WIFI RECONNECT
// --------------------------------------------------

void ensureWiFi() {

    // WiFi is currently connected.
    if (
        WiFi.status()
        == WL_CONNECTED
    ) {

        // Detect a new reconnection.
        if (!wifiWasConnected) {

            wifiWasConnected = true;

            Serial.println();
            Serial.println(
                "WiFi reconnected!"
            );

            printNetworkInfo();
        }

        return;
    }


    // Detect initial disconnection.
    if (wifiWasConnected) {

        wifiWasConnected = false;

        Serial.println();
        Serial.println(
            "WiFi disconnected."
        );
    }


    // Do not block for 10 seconds.
    // Simply attempt another connection
    // every 5 seconds.
    if (
        millis()
            - lastWiFiReconnectAttempt
        >= WIFI_RECONNECT_INTERVAL
    ) {

        lastWiFiReconnectAttempt =
            millis();


        Serial.println(
            "Attempting WiFi reconnect..."
        );


        WiFi.begin(
            WIFI_SSID,
            WIFI_PASSWORD
        );
    }
}


// --------------------------------------------------
// NETWORK INFORMATION
// --------------------------------------------------

void printNetworkInfo() {

    if (
        WiFi.status()
        != WL_CONNECTED
    ) {

        return;
    }


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

    Serial.println("1) Eastern");
    Serial.println("2) Central");
    Serial.println("3) Mountain");
    Serial.println("4) Pacific");
    Serial.println("5) Alaska");
    Serial.println("6) Hawaii");
    Serial.println("7) Atlantic");

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
// GET CURRENT TIME FROM TIMEAPI.IO
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
// SUBTRACT ELAPSED SECONDS FROM TIMESTAMP
// --------------------------------------------------

String subtractSecondsFromTimestamp(
    const String& timestamp,
    unsigned long seconds
) {

    int year;
    int month;
    int day;

    int hour;
    int minute;
    int second;


    int result =
        sscanf(
            timestamp.c_str(),
            "%d-%d-%d %d:%d:%d",
            &year,
            &month,
            &day,
            &hour,
            &minute,
            &second
        );


    if (result != 6) {

        return "";
    }


    /*
     * We use UTC here only as a convenient way
     * to perform calendar arithmetic.
     *
     * The timestamp returned by TimeAPI already
     * represents the selected user's time zone.
     */

    setenv(
        "TZ",
        "UTC0",
        1
    );

    tzset();


    struct tm timeInfo = {};

    timeInfo.tm_year =
        year - 1900;

    timeInfo.tm_mon =
        month - 1;

    timeInfo.tm_mday =
        day;

    timeInfo.tm_hour =
        hour;

    timeInfo.tm_min =
        minute;

    timeInfo.tm_sec =
        second;

    timeInfo.tm_isdst =
        0;


    time_t currentTime =
        mktime(
            &timeInfo
        );


    if (currentTime == -1) {

        return "";
    }


    // Move backward by the amount of time
    // that passed since the sensor reading.
    currentTime -= seconds;


    struct tm* corrected =
        gmtime(
            &currentTime
        );


    if (corrected == nullptr) {

        return "";
    }


    char buffer[20];


    strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%d %H:%M:%S",
        corrected
    );


    return String(buffer);
}


// --------------------------------------------------
// CREATE ORIGINAL TIMESTAMP AFTER WIFI RETURNS
// --------------------------------------------------

bool preparePendingTimestamp() {

    if (!pending.active) {

        return false;
    }


    if (pending.timestampReady) {

        return true;
    }


    if (
        WiFi.status()
        != WL_CONNECTED
    ) {

        return false;
    }


    Serial.println();
    Serial.println(
        "WiFi available."
    );

    Serial.println(
        "Obtaining current server time..."
    );


    String currentServerTime =
        getCurrentTime();


    if (currentServerTime == "") {

        Serial.println(
            "Could not obtain server time."
        );

        return false;
    }


    // Unsigned subtraction handles millis()
    // rollover correctly for normal intervals.
    unsigned long elapsedMilliseconds =
        millis()
        - pending.measurementMillis;


    unsigned long elapsedSeconds =
        elapsedMilliseconds / 1000UL;


    Serial.print(
        "Current Server Time: "
    );

    Serial.println(
        currentServerTime
    );


    Serial.print(
        "Elapsed Since Measurement: "
    );

    Serial.print(
        elapsedSeconds
    );

    Serial.println(
        " seconds"
    );


    String reconstructedTimestamp =
        subtractSecondsFromTimestamp(
            currentServerTime,
            elapsedSeconds
        );


    if (
        reconstructedTimestamp == ""
    ) {

        Serial.println(
            "Could not reconstruct timestamp."
        );

        return false;
    }


    pending.timestamp =
        reconstructedTimestamp;

    pending.timestampReady =
        true;


    Serial.print(
        "Reconstructed Original Timestamp: "
    );

    Serial.println(
        pending.timestamp
    );


    return true;
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

    // Keep the existing behavior:
    // only one undelivered measurement
    // may be pending at a time.
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


    /*
     * Record the exact millis() value BEFORE
     * doing network operations.
     *
     * This represents when the measurement
     * was physically triggered.
     */
    unsigned long measurementTime =
        millis();


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


    // Save measurement immediately.
    pending.active =
        true;

    pending.node =
        node;

    pending.lightVoltage =
        lightVoltage;

    pending.distance =
        distance;

    pending.measurementMillis =
        measurementTime;

    pending.lastAttempt =
        0;

    pending.timestamp =
        "";

    pending.timestampReady =
        false;


    Serial.print(
        "Node: "
    );

    Serial.println(
        pending.node
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


    // ------------------------------------------------
    // WIFI IS AVAILABLE
    // ------------------------------------------------

    if (
        WiFi.status()
        == WL_CONNECTED
    ) {

        Serial.print(
            "RSSI: "
        );

        Serial.print(
            WiFi.RSSI()
        );

        Serial.println(
            " dBm"
        );


        /*
         * Since WiFi is currently working,
         * obtain the timestamp immediately.
         */
        String timestamp =
            getCurrentTime();


        if (timestamp != "") {

            pending.timestamp =
                timestamp;

            pending.timestampReady =
                true;


            Serial.print(
                "Timestamp: "
            );

            Serial.println(
                pending.timestamp
            );


            sendPendingMessage();

            return;
        }


        /*
         * Time server failed even though
         * WiFi is connected.
         *
         * Keep the measurement and reconstruct
         * its timestamp later.
         */
        Serial.println(
            "Time server unavailable."
        );

        Serial.println(
            "Measurement saved using elapsed time."
        );


        return;
    }


    // ------------------------------------------------
    // WIFI IS OFF
    // ------------------------------------------------

    Serial.println(
        "WiFi is unavailable."
    );

    Serial.println(
        "Measurement saved locally."
    );

    Serial.println(
        "Waiting for WiFi to return..."
    );

    Serial.println(
        "Original time will be reconstructed"
    );

    Serial.println(
        "using elapsed milliseconds."
    );
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


    if (
        WiFi.status()
        != WL_CONNECTED
    ) {

        Serial.println(
            "Cannot transmit: WiFi unavailable."
        );

        return false;
    }


    /*
     * Before transmitting, make sure the
     * measurement has its original timestamp.
     */
    if (!pending.timestampReady) {

        if (!preparePendingTimestamp()) {

            return false;
        }
    }


    pending.lastAttempt =
        millis();


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


    // New record successfully stored.
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


    // Server already contains the exact
    // node + timestamp record.
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


    /*
     * ------------------------------------------------
     * OFFLINE MEASUREMENT
     * ------------------------------------------------
     *
     * If this measurement does not yet have a real
     * timestamp, send it immediately after WiFi
     * becomes available.
     */

    if (!pending.timestampReady) {

        if (
            WiFi.status()
            == WL_CONNECTED
        ) {

            Serial.println();
            Serial.println(
                "WiFi restored."
            );

            Serial.println(
                "Reconstructing original timestamp..."
            );


            if (
                preparePendingTimestamp()
            ) {

                sendPendingMessage();
            }
        }


        return;
    }


    /*
     * ------------------------------------------------
     * NORMAL RETRANSMISSION
     * ------------------------------------------------
     *
     * Once the timestamp exists, retry the exact
     * same message every 30 seconds.
     */

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
