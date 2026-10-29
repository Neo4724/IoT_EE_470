<?php

ini_set('display_errors', 1);
error_reporting(E_ALL);

// Database connection
$hostname = "localhost";
$username = "u863989605_db_AntonioKass";
$password = "Blastwing472";
$database = "u863989605_AntonioKassis";

$conn = new mysqli($hostname, $username, $password, $database);

if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}


// ---------------------------------
// Get Base64 encoded query string
// ---------------------------------

$encoded = $_SERVER['QUERY_STRING'];

if (empty($encoded)) {
    die("ERROR: No encoded data received.");
}


// ---------------------------------
// Decode Base64
// ---------------------------------

$decoded = base64_decode($encoded, true);

if ($decoded === false) {
    die("ERROR: Invalid Base64 message.");
}


// ---------------------------------
// Parse decoded values
// ---------------------------------

parse_str($decoded, $values);

$nodeId = $values["nodeId"] ?? null;
$nodeTemp = $values["nodeTemp"] ?? null;
$nodeHumidity = $values["nodeHumidity"] ?? null;
$timeReceived = $values["timeReceived"] ?? null;


// ---------------------------------
// Show decoded values
// ---------------------------------

echo "<h2>Decoded Data</h2>";

echo "Node ID: " . htmlspecialchars($nodeId) . "<br>";
echo "Temperature: " . htmlspecialchars($nodeTemp) . "<br>";
echo "Humidity: " . htmlspecialchars($nodeHumidity) . "<br>";
echo "Time Received: " . htmlspecialchars($timeReceived) . "<br><br>";


// ---------------------------------
// Check required data
// ---------------------------------

if (
    $nodeId === null ||
    $nodeTemp === null ||
    $nodeHumidity === null ||
    $timeReceived === null
) {
    die("ERROR: Missing required value.");
}


// ---------------------------------
// Validate ranges
// ---------------------------------

if (!is_numeric($nodeTemp) || $nodeTemp < -10 || $nodeTemp > 100) {
    die("ERROR: Invalid temperature.");
}

if (!is_numeric($nodeHumidity) || $nodeHumidity < 0 || $nodeHumidity > 100) {
    die("ERROR: Invalid humidity.");
}


// ---------------------------------
// Check node registration
// ---------------------------------

$checkNode = $conn->prepare(
    "SELECT node_name
     FROM sensor_register
     WHERE node_name = ?"
);

$checkNode->bind_param("s", $nodeId);
$checkNode->execute();

$nodeResult = $checkNode->get_result();

if ($nodeResult->num_rows == 0) {
    die("ERROR: Node is not registered.");
}


// ---------------------------------
// Check duplicate time
// ---------------------------------

$checkDuplicate = $conn->prepare(
    "SELECT node_name
     FROM sensor_data
     WHERE node_name = ?
     AND time_received = ?"
);

$checkDuplicate->bind_param(
    "ss",
    $nodeId,
    $timeReceived
);

$checkDuplicate->execute();

$duplicateResult = $checkDuplicate->get_result();

if ($duplicateResult->num_rows > 0) {
    die("ERROR: Duplicate timestamp.");
}


// ---------------------------------
// Insert into database
// ---------------------------------

$insert = $conn->prepare(
    "INSERT INTO sensor_data
    (
        node_name,
        time_received,
        temperature,
        humidity
    )
    VALUES (?, ?, ?, ?)"
);

$insert->bind_param(
    "ssdd",
    $nodeId,
    $timeReceived,
    $nodeTemp,
    $nodeHumidity
);

if ($insert->execute()) {
    echo "<h3>Data inserted successfully.</h3>";
} else {
    echo "Insert failed: " . $insert->error;
}


// Close
$checkNode->close();
$checkDuplicate->close();
$insert->close();
$conn->close();

?>