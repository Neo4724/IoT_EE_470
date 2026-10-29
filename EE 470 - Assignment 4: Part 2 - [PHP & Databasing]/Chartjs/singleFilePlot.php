<?php

header('Content-Type: application/json');

// Your Hostinger database information
define('DB_HOST', 'localhost');
define('DB_USERNAME', 'u863989605_db_AntonioKass');
define('DB_PASSWORD', 'B1@stwing472');
define('DB_NAME', 'u863989605_AntonioKassis');

// Connect to database
$mysqli = new mysqli(
    DB_HOST,
    DB_USERNAME,
    DB_PASSWORD,
    DB_NAME
);

// Check connection
if ($mysqli->connect_error) {
    die("Connection failed: " . $mysqli->connect_error);
}

// Get data from one sensor
$query = "
    SELECT node_name, temperature, humidity, time_received
    FROM sensor_data
    WHERE node_name = 'node_1'
    ORDER BY time_received
";

$result = $mysqli->query($query);

// Store results
$data = array();

while ($row = $result->fetch_assoc()) {
    $data[] = $row;
}

// Close connection
$mysqli->close();

// Output JSON
echo json_encode($data);

?>