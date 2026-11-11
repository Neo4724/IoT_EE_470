<?php

header("Content-Type: text/plain");


// --------------------------------------------------
// DATABASE CONNECTION
// --------------------------------------------------

$hostname = "localhost";
$username = "u863989605_db_AntonioKass";
$password = "Blastwing472";
$database = "u863989605_AntonioKassis";


$conn = new mysqli(
    $hostname,
    $username,
    $password,
    $database
);


if ($conn->connect_error) {

    http_response_code(500);

    die("ERROR: DATABASE_CONNECTION");
}


// --------------------------------------------------
// GET PARAMETERS
// --------------------------------------------------

$node = $_GET["nodeId"] ?? null;
$time = $_GET["timeReceived"] ?? null;
$light = $_GET["lightVoltage"] ?? null;
$distance = $_GET["distanceCM"] ?? null;


// --------------------------------------------------
// CHECK REQUIRED PARAMETERS
// --------------------------------------------------

if (
    $node === null ||
    $time === null ||
    $light === null ||
    $distance === null
) {

    http_response_code(400);

    die("ERROR: MISSING_PARAMETER");
}


// Timestamp MUST come from ESP
if (trim($time) === "") {

    http_response_code(400);

    die("ERROR: MISSING_TIMESTAMP");
}


// --------------------------------------------------
// VALIDATE SENSOR VALUES
// --------------------------------------------------

if (!is_numeric($light)) {

    http_response_code(400);

    die("ERROR: INVALID_LIGHT");
}


if (
    $light < 0 ||
    $light > 3.3
) {

    http_response_code(400);

    die("ERROR: LIGHT_OUT_OF_RANGE");
}


if (!is_numeric($distance)) {

    http_response_code(400);

    die("ERROR: INVALID_DISTANCE");
}


if (
    $distance < 0 ||
    $distance > 500
) {

    http_response_code(400);

    die("ERROR: DISTANCE_OUT_OF_RANGE");
}


// --------------------------------------------------
// CHECK REGISTERED NODE
// --------------------------------------------------

$checkNode = $conn->prepare(
    "SELECT node_name
     FROM sensor_register
     WHERE node_name = ?"
);


$checkNode->bind_param(
    "s",
    $node
);


$checkNode->execute();


$result =
    $checkNode->get_result();


if ($result->num_rows == 0) {

    http_response_code(403);

    die("ERROR: NODE_NOT_REGISTERED");
}


// --------------------------------------------------
// CHECK DUPLICATE TIMESTAMP
// --------------------------------------------------

$duplicate = $conn->prepare(
    "SELECT node_name
     FROM sensor_data
     WHERE node_name = ?
     AND time_received = ?"
);


$duplicate->bind_param(
    "ss",
    $node,
    $time
);


$duplicate->execute();


$duplicateResult =
    $duplicate->get_result();


if ($duplicateResult->num_rows > 0) {

    // Already stored = successful delivery
    http_response_code(200);

    die("DUPLICATE");
}


// --------------------------------------------------
// BEGIN TRANSACTION
// --------------------------------------------------

$conn->begin_transaction();


// --------------------------------------------------
// INSERT SENSOR DATA
// --------------------------------------------------

$insert = $conn->prepare(
    "INSERT INTO sensor_data
    (
        node_name,
        time_received,
        light_voltage,
        distance_cm
    )
    VALUES (?, ?, ?, ?)"
);


$insert->bind_param(
    "ssdd",
    $node,
    $time,
    $light,
    $distance
);


if (!$insert->execute()) {

    $conn->rollback();

    http_response_code(500);

    die("ERROR: INSERT_FAILED");
}


// --------------------------------------------------
// UPDATE SENSOR ACTIVITY
// --------------------------------------------------

$activity = $conn->prepare(
    "INSERT INTO sensor_activity
    (
        node_name,
        message_count
    )
    VALUES (?, 1)

    ON DUPLICATE KEY UPDATE
        message_count =
        message_count + 1"
);


$activity->bind_param(
    "s",
    $node
);


if (!$activity->execute()) {

    $conn->rollback();

    http_response_code(500);

    die("ERROR: ACTIVITY_UPDATE_FAILED");
}


// --------------------------------------------------
// COMMIT
// --------------------------------------------------

$conn->commit();


echo "STORED";


$conn->close();

?>