<?php

ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

// Database connection information
$hostname = "localhost";
$username = "u863989605_db_AntonioKass";
$password = "Blastwing472";
$database = "u863989605_AntonioKassis";

// Connect to database
$conn = new mysqli(
    $hostname,
    $username,
    $password,
    $database
);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}


// --------------------------------------------------
// Get values from URL
// --------------------------------------------------

$nodeId = $_GET["nodeId"] ?? null;
$timeReceived = $_GET["timeReceived"] ?? null;
$nodeTemp = $_GET["nodeTemp"] ?? null;
$nodeHumidity = $_GET["nodeHumidity"] ?? null;


// --------------------------------------------------
// Check required values
// --------------------------------------------------

if ($nodeId === null) {
    die("ERROR: nodeId is required.");
}

if ($nodeTemp === null) {
    die("ERROR: nodeTemp is required.");
}

if ($nodeHumidity === null) {
    die("ERROR: nodeHumidity is required.");
}


// --------------------------------------------------
// Validate numeric values
// --------------------------------------------------

if (!is_numeric($nodeTemp)) {
    die("ERROR: Temperature must be numeric.");
}

if (!is_numeric($nodeHumidity)) {
    die("ERROR: Humidity must be numeric.");
}

$nodeTemp = floatval($nodeTemp);
$nodeHumidity = floatval($nodeHumidity);


// --------------------------------------------------
// Check allowed ranges
// --------------------------------------------------

if ($nodeTemp < -10 || $nodeTemp > 100) {
    die("ERROR: Temperature must be between -10 and 100 C.");
}

if ($nodeHumidity < 0 || $nodeHumidity > 100) {
    die("ERROR: Humidity must be between 0 and 100 percent.");
}


// --------------------------------------------------
// If time is not supplied, create timestamp
// --------------------------------------------------

if ($timeReceived === null || $timeReceived === "") {

    $timeReceived = date("Y-m-d H:i:s");

}


// --------------------------------------------------
// Check whether node is registered
// --------------------------------------------------

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


// --------------------------------------------------
// Check for duplicate timestamp
// --------------------------------------------------

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

    die(
        "ERROR: Duplicate timestamp for this node."
    );

}


// --------------------------------------------------
// Insert data
// --------------------------------------------------

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

    echo "<h2>Data inserted successfully</h2>";

    echo "<p>Node: "
        . htmlspecialchars($nodeId)
        . "</p>";

    echo "<p>Time: "
        . htmlspecialchars($timeReceived)
        . "</p>";

    echo "<p>Temperature: "
        . htmlspecialchars($nodeTemp)
        . " C</p>";

    echo "<p>Humidity: "
        . htmlspecialchars($nodeHumidity)
        . " %</p>";

} else {

    echo "ERROR: " . $insert->error;

}


// --------------------------------------------------
// Close connection
// --------------------------------------------------

$checkNode->close();
$checkDuplicate->close();
$insert->close();
$conn->close();

?>