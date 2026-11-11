<?php

ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

header("Content-Type: application/json");


// --------------------------------------------------
// DATABASE CONNECTION
// --------------------------------------------------

$servername = "localhost";
$username = "u863989605_db_AntonioKass";
$password = "Blastwing472";
$dbname = "u863989605_AntonioKassis";


$conn = new mysqli(
    $servername,
    $username,
    $password,
    $dbname
);


if ($conn->connect_error) {

    http_response_code(500);

    echo json_encode(
        array(
            "error" =>
                "Database connection failed"
        )
    );

    exit;
}


// --------------------------------------------------
// SELECTED NODE
// --------------------------------------------------

$node =
    $_GET["node"]
    ?? "node_1";


// --------------------------------------------------
// SENSOR DATA
// --------------------------------------------------

$query = "
    SELECT
        node_name,
        time_received,
        temperature,
        humidity,
        light_voltage,
        distance_cm

    FROM sensor_data

    WHERE node_name = ?

    ORDER BY time_received
";


$stmt = $conn->prepare($query);

$stmt->bind_param(
    "s",
    $node
);

$stmt->execute();

$result =
    $stmt->get_result();


$sensorData =
    array();


while (
    $row =
    $result->fetch_assoc()
) {

    $sensorData[] = $row;
}


// --------------------------------------------------
// STATISTICS
// --------------------------------------------------

$statsQuery = "
    SELECT

        AVG(temperature)
            AS avg_temperature,

        MIN(temperature)
            AS min_temperature,

        MAX(temperature)
            AS max_temperature,


        AVG(humidity)
            AS avg_humidity,

        MIN(humidity)
            AS min_humidity,

        MAX(humidity)
            AS max_humidity,


        AVG(light_voltage)
            AS avg_light,

        MIN(light_voltage)
            AS min_light,

        MAX(light_voltage)
            AS max_light,


        AVG(distance_cm)
            AS avg_distance,

        MIN(distance_cm)
            AS min_distance,

        MAX(distance_cm)
            AS max_distance

    FROM sensor_data

    WHERE node_name = ?
";


$statsStatement =
    $conn->prepare(
        $statsQuery
    );


$statsStatement->bind_param(
    "s",
    $node
);


$statsStatement->execute();


$statsResult =
    $statsStatement->get_result();


$stats =
    $statsResult->fetch_assoc();


// --------------------------------------------------
// MESSAGE COUNT
// --------------------------------------------------

$countQuery = "
    SELECT
        message_count

    FROM sensor_activity

    WHERE node_name = ?
";


$countStatement =
    $conn->prepare(
        $countQuery
    );


$countStatement->bind_param(
    "s",
    $node
);


$countStatement->execute();


$countResult =
    $countStatement->get_result();


$countRow =
    $countResult->fetch_assoc();


$messageCount =
    $countRow["message_count"]
    ?? 0;


// --------------------------------------------------
// OUTPUT
// --------------------------------------------------

$output =
    array(

        "node" =>
            $node,

        "sensorData" =>
            $sensorData,

        "stats" =>
            $stats,

        "messageCount" =>
            $messageCount
    );


echo json_encode(
    $output
);


$conn->close();

?>