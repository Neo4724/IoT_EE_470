import paho.mqtt.client as mqtt
import mysql.connector
from mysql.connector import Error
from datetime import datetime
from zoneinfo import ZoneInfo


# ==================================================
# MQTT SETTINGS
# ==================================================

BROKER = "broker.mqttdashboard.com"
PORT = 8883

TOPIC = "testtopic/temp/outTopic/akassis/pot"


# ==================================================
# DATABASE SETTINGS
# ==================================================

DB_HOST = "srv748.hstgr.io"
DB_USER = "u863989605_db_AntonioKass"
DB_PASSWORD = "Blastwing472"
DB_NAME = "u863989605_AntonioKassis"
DB_PORT = 3306


# ==================================================
# PUSH VALUE TO DATABASE
# ==================================================

def push_to_database(voltage, timestamp):

    connection = None
    cursor = None

    try:

        print("Connecting to database...")

        connection = mysql.connector.connect(
            host=DB_HOST,
            user=DB_USER,
            password=DB_PASSWORD,
            database=DB_NAME,
            port=DB_PORT
        )

        if connection.is_connected():

            print("Connected to database!")

            cursor = connection.cursor()

            query = """
                INSERT INTO mqtt_potentiometer
                (voltage, time_received)
                VALUES (%s, %s)
            """

            cursor.execute(
                query,
                (voltage, timestamp)
            )

            connection.commit()

            print(
                f"Inserted {voltage:.3f} V "
                f"at {timestamp} California time."
            )

    except Error as error:

        print("DATABASE ERROR:")
        print(error)

    finally:

        if cursor is not None:
            cursor.close()

        if (
            connection is not None
            and connection.is_connected()
        ):
            connection.close()
            print("Database connection closed.")


# ==================================================
# MQTT CONNECT CALLBACK
# ==================================================

def on_connect(client, userdata, flags, rc):

    if rc == 0:

        print()
        print("Connected to MQTT broker!")

        client.subscribe(TOPIC)

        print(
            f"Subscribed to: {TOPIC}"
        )

    else:

        print(
            f"MQTT connection failed. Return code: {rc}"
        )


# ==================================================
# MQTT MESSAGE CALLBACK
# ==================================================

def on_message(client, userdata, msg):

    print()
    print("MQTT MESSAGE RECEIVED")
    print("---------------------")

    print(
        f"Topic: {msg.topic}"
    )

    payload = (
        msg.payload
        .decode()
        .strip()
    )

    print(
        f"Payload: {payload}"
    )

    try:

        voltage = float(payload)

        # --------------------------------------------------
        # CREATE CALIFORNIA TIMESTAMP BEFORE DATABASE INSERT
        # --------------------------------------------------

        california_time = datetime.now(
            ZoneInfo(
                "America/Los_Angeles"
            )
        )

        timestamp = california_time.strftime(
            "%Y-%m-%d %H:%M:%S"
        )

        print(
            f"Potentiometer Voltage: "
            f"{voltage:.3f} V"
        )

        print(
            f"California Timestamp: "
            f"{timestamp}"
        )

        # --------------------------------------------------
        # PUSH BOTH VALUES TO DATABASE
        # --------------------------------------------------

        push_to_database(
            voltage,
            timestamp
        )

    except ValueError:

        print(
            "ERROR: Payload is not a valid number."
        )


# ==================================================
# CREATE MQTT CLIENT
# ==================================================

client = mqtt.Client()

client.on_connect = on_connect
client.on_message = on_message


# ==================================================
# TLS SETTINGS
# ==================================================

client.tls_set()

client.tls_insecure_set(
    True
)


# ==================================================
# CONNECT TO MQTT
# ==================================================

print(
    "Connecting to MQTT broker..."
)

client.connect(
    BROKER,
    PORT,
    60
)


# ==================================================
# RUN MQTT CLIENT
# ==================================================

client.loop_forever()