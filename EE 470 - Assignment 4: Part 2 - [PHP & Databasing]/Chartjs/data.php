<?php

header('Content-Type: application/json');

// Your Hostinger database information
define('DB_HOST', 'localhost');
define('DB_USERNAME', 'u863989605_db_AntonioKass');
define('DB_PASSWORD', 'Blastwing472');
define('DB_NAME', 'u863989605_AntonioKassis');

$mysqli = new mysqli(
    DB_HOST,
    DB_USERNAME,
    DB_PASSWORD,
    DB_NAME
);

if ($mysqli->connect_error) {
    die("Connection failed: " . $mysqli->connect_error);
}

// Get selected node from URL
$node = $_GET["node"] ?? "node_1";

// Get sensor data
$query = "
    SELECT
        node_name,
        time_received,
        temperature,
        humidity
    FROM sensor_data
    WHERE node_name = ?
    ORDER BY time_received
";

$stmt = $mysqli->prepare($query);
$stmt->bind_param("s", $node);
$stmt->execute();

$result = $stmt->get_result();

$data = array();

while ($row = $result->fetch_assoc()) {
    $data[] = $row;
}

// Get statistics
$statsQuery = "
    SELECT
        AVG(temperature) AS avg_temperature,
        AVG(humidity) AS avg_humidity,
        MAX(temperature) AS max_temperature,
        MIN(humidity) AS min_humidity
    FROM sensor_data
    WHERE node_name = ?
";

$stmtStats = $mysqli->prepare($statsQuery);
$stmtStats->bind_param("s", $node);
$stmtStats->execute();

$statsResult = $stmtStats->get_result();
$stats = $statsResult->fetch_assoc();

// Return JSON
$output = array(
    "node" => $node,
    "sensorData" => $data,
    "stats" => $stats
);

echo json_encode($output);

$stmt->close();
$stmtStats->close();
$mysqli->close();

?>