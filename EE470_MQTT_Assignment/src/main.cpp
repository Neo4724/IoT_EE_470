#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <PubSubClient.h>

// --------------------------------------------------
// WIFI
// --------------------------------------------------

const char* WIFI_SSID =
    "Samsung Galaxy A10e 8801";

const char* WIFI_PASSWORD =
    "0pp0pp0pp";

// --------------------------------------------------
// MQTT BROKER
// --------------------------------------------------

const char* MQTT_BROKER =
    "broker.mqttdashboard.com";

const int MQTT_PORT = 8883;

// --------------------------------------------------
// MQTT TOPICS
// --------------------------------------------------

// Potentiometer publishes here
const char* POT_TOPIC =
    "testtopic/temp/outTopic/akassis/pot";

// Button publishes here
const char* BUTTON_TOPIC =
    "ee470/emanuel/led";

// ESP subscribes here for LED commands
const char* LED_TOPIC =
    "testtopic/temp/inTopic/akassis";

// --------------------------------------------------
// HARDWARE
// --------------------------------------------------

#define POT_PIN A0
#define LED_PIN D5
#define BUTTON_PIN D6

// --------------------------------------------------
// MQTT OBJECTS
// --------------------------------------------------

BearSSL::WiFiClientSecure secureClient;

PubSubClient mqttClient(secureClient);

// --------------------------------------------------
// POTENTIOMETER TIMING
// --------------------------------------------------

const unsigned long POT_PUBLISH_INTERVAL =
    15000;

unsigned long lastPotPublishTime = 0;

// --------------------------------------------------
// BUTTON TIMING
// --------------------------------------------------

bool previousButtonState = HIGH;

bool buttonEventActive = false;

unsigned long buttonPressTime = 0;

const unsigned long BUTTON_RESET_TIME =
    5000;

// --------------------------------------------------
// BUTTON DEBOUNCE
// --------------------------------------------------

unsigned long lastButtonEvent = 0;

const unsigned long DEBOUNCE_TIME =
    250;

// --------------------------------------------------
// FUNCTION PROTOTYPES
// --------------------------------------------------

void connectWiFi();

void connectMQTT();

void mqttCallback(
    char* topic,
    byte* payload,
    unsigned int length
);

void publishPotentiometer();

void checkButton();

void publishButtonValue(
    const char* value
);

// --------------------------------------------------
// SETUP
// --------------------------------------------------

void setup() {

    Serial.begin(115200);

    delay(1000);

    pinMode(
        LED_PIN,
        OUTPUT
    );

    pinMode(
        BUTTON_PIN,
        INPUT_PULLUP
    );

    digitalWrite(
        LED_PIN,
        LOW
    );

    Serial.println();

    Serial.println(
        "EE 470 MQTT System"
    );

    Serial.println(
        "------------------"
    );

    Serial.println();

    connectWiFi();

    // TLS encryption without certificate verification
    secureClient.setInsecure();

    mqttClient.setServer(
        MQTT_BROKER,
        MQTT_PORT
    );

    mqttClient.setCallback(
        mqttCallback
    );

    connectMQTT();
}

// --------------------------------------------------
// MAIN LOOP
// --------------------------------------------------

void loop() {

    // --------------------------------------------------
    // WIFI RECONNECT
    // --------------------------------------------------

    if (
        WiFi.status()
        != WL_CONNECTED
    ) {

        connectWiFi();
    }

    // --------------------------------------------------
    // MQTT RECONNECT
    // --------------------------------------------------

    if (
        !mqttClient.connected()
    ) {

        connectMQTT();
    }

    // --------------------------------------------------
    // KEEP MQTT CONNECTION ACTIVE
    // --------------------------------------------------

    mqttClient.loop();

    // --------------------------------------------------
    // CHECK PHYSICAL BUTTON
    // --------------------------------------------------

    checkButton();

    // --------------------------------------------------
    // PUBLISH POTENTIOMETER EVERY 15 SECONDS
    // --------------------------------------------------

    if (
        millis() - lastPotPublishTime
        >= POT_PUBLISH_INTERVAL
    ) {

        lastPotPublishTime =
            millis();

        publishPotentiometer();
    }
}

// --------------------------------------------------
// CONNECT TO WIFI
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

    Serial.print(
        "IP Address: "
    );

    Serial.println(
        WiFi.localIP()
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
// CONNECT TO MQTT BROKER
// --------------------------------------------------

void connectMQTT() {

    while (
        !mqttClient.connected()
    ) {

        Serial.println();

        Serial.print(
            "Connecting to MQTT broker..."
        );

        // Create unique client ID
        String clientId =
            "ESP8266-AK-";

        clientId +=
            String(
                random(0xffff),
                HEX
            );

        if (
            mqttClient.connect(
                clientId.c_str()
            )
        ) {

            Serial.println(
                "connected!"
            );

            // Subscribe to LED command topic
            bool subscribed =
                mqttClient.subscribe(
                    LED_TOPIC
                );

            if (subscribed) {

                Serial.print(
                    "Subscribed to LED topic: "
                );

                Serial.println(
                    LED_TOPIC
                );

            } else {

                Serial.println(
                    "LED topic subscription failed!"
                );
            }

        } else {

            Serial.print(
                "MQTT connection failed. State = "
            );

            Serial.println(
                mqttClient.state()
            );

            Serial.println(
                "Retrying in 5 seconds..."
            );

            delay(5000);
        }
    }
}

// --------------------------------------------------
// MQTT CALLBACK
// --------------------------------------------------

void mqttCallback(
    char* topic,
    byte* payload,
    unsigned int length
) {

    Serial.println();

    Serial.println(
        "MQTT MESSAGE RECEIVED"
    );

    Serial.println(
        "---------------------"
    );

    Serial.print(
        "Topic: "
    );

    Serial.println(
        topic
    );

    String message = "";

    for (
        unsigned int i = 0;
        i < length;
        i++
    ) {

        message +=
            (char)payload[i];
    }

    Serial.print(
        "Payload: "
    );

    Serial.println(
        message
    );

    // --------------------------------------------------
    // LED CONTROL
    // --------------------------------------------------

    if (
        String(topic)
        == LED_TOPIC
    ) {

        if (
            message == "1"
        ) {

            digitalWrite(
                LED_PIN,
                HIGH
            );

            Serial.println(
                "LED ON"
            );
        }

        else if (
            message == "0"
        ) {

            digitalWrite(
                LED_PIN,
                LOW
            );

            Serial.println(
                "LED OFF"
            );
        }

        else {

            Serial.println(
                "Unknown LED command."
            );
        }
    }
}

// --------------------------------------------------
// PUBLISH POTENTIOMETER
// --------------------------------------------------

void publishPotentiometer() {

    int rawValue =
        analogRead(
            POT_PIN
        );

    float voltage =
        rawValue *
        (3.3 / 1023.0);

    char payload[16];

    dtostrf(
        voltage,
        1,
        3,
        payload
    );

    Serial.println();

    Serial.println(
        "Publishing Potentiometer"
    );

    Serial.println(
        "------------------------"
    );

    Serial.print(
        "Raw Value: "
    );

    Serial.println(
        rawValue
    );

    Serial.print(
        "Voltage: "
    );

    Serial.print(
        voltage,
        3
    );

    Serial.println(
        " V"
    );

    Serial.print(
        "Topic: "
    );

    Serial.println(
        POT_TOPIC
    );

    Serial.print(
        "Payload: "
    );

    Serial.println(
        payload
    );

    bool success =
        mqttClient.publish(
            POT_TOPIC,
            payload
        );

    if (success) {

        Serial.println(
            "Potentiometer publish successful!"
        );

    } else {

        Serial.println(
            "Potentiometer publish failed!"
        );
    }
}

// --------------------------------------------------
// CHECK PHYSICAL BUTTON
// --------------------------------------------------

void checkButton() {

    bool currentButtonState =
        digitalRead(
            BUTTON_PIN
        );

    // --------------------------------------------------
    // NEW BUTTON PRESS
    // --------------------------------------------------

    if (
        currentButtonState == LOW &&
        previousButtonState == HIGH &&
        millis() - lastButtonEvent
            > DEBOUNCE_TIME
    ) {

        lastButtonEvent =
            millis();

        Serial.println();

        Serial.println(
            "BUTTON PRESSED"
        );

        // Publish 1 immediately
        publishButtonValue(
            "1"
        );

        // Start 5 second timer
        buttonPressTime =
            millis();

        buttonEventActive =
            true;
    }

    previousButtonState =
        currentButtonState;

    // --------------------------------------------------
    // AFTER 5 SECONDS, PUBLISH 0
    // --------------------------------------------------

    if (
        buttonEventActive &&
        millis() - buttonPressTime
            >= BUTTON_RESET_TIME
    ) {

        Serial.println();

        Serial.println(
            "5 seconds elapsed."
        );

        publishButtonValue(
            "0"
        );

        buttonEventActive =
            false;
    }
}

// --------------------------------------------------
// PUBLISH BUTTON VALUE
// --------------------------------------------------

void publishButtonValue(
    const char* value
) {

    Serial.println();

    Serial.println(
        "Publishing Button"
    );

    Serial.println(
        "-----------------"
    );

    Serial.print(
        "Button Value: "
    );

    Serial.println(
        value
    );

    Serial.print(
        "Topic: "
    );

    Serial.println(
        BUTTON_TOPIC
    );

    bool success =
        mqttClient.publish(
            BUTTON_TOPIC,
            value
        );

    if (success) {

        Serial.println(
            "Button publish successful!"
        );

    } else {

        Serial.println(
            "Button publish failed!"
        );
    }
}