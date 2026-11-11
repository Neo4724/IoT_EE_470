<?php

ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);


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
    die(
        "Database connection failed: "
        . $conn->connect_error
    );
}


// --------------------------------------------------
// REGISTERED SENSORS
// --------------------------------------------------

$sqlRegister = "
    SELECT
        node_name,
        manufacturer,
        longitude,
        latitude
    FROM sensor_register
    ORDER BY node_name
";

$resultRegister = $conn->query($sqlRegister);


// --------------------------------------------------
// SENSOR DATA
// --------------------------------------------------

$sqlData = "
    SELECT
        node_name,
        time_received,
        temperature,
        humidity,
        light_voltage,
        distance_cm
    FROM sensor_data
    ORDER BY node_name, time_received
";

$resultData = $conn->query($sqlData);


// --------------------------------------------------
// SENSOR ACTIVITY
// --------------------------------------------------

$sqlActivity = "
    SELECT
        node_name,
        message_count
    FROM sensor_activity
    ORDER BY node_name
";

$resultActivity = $conn->query($sqlActivity);


// --------------------------------------------------
// NODE 1 STATISTICS
// --------------------------------------------------

$sqlStats = "
    SELECT

        AVG(temperature) AS avg_temperature,
        MIN(temperature) AS min_temperature,
        MAX(temperature) AS max_temperature,

        AVG(humidity) AS avg_humidity,
        MIN(humidity) AS min_humidity,
        MAX(humidity) AS max_humidity,

        AVG(light_voltage) AS avg_light,
        MIN(light_voltage) AS min_light,
        MAX(light_voltage) AS max_light,

        AVG(distance_cm) AS avg_distance,
        MIN(distance_cm) AS min_distance,
        MAX(distance_cm) AS max_distance

    FROM sensor_data

    WHERE node_name = 'node_1'
";

$resultStats = $conn->query($sqlStats);

$stats = $resultStats->fetch_assoc();

?>

<!DOCTYPE html>
<html>

<head>

<meta charset="UTF-8">

<title>EE 470 IoT Sensor Database</title>

<style>

body {
    font-family: Arial, sans-serif;
    background-color: #f4f4f4;
    margin: 30px;
}

h1 {
    text-align: center;
    color: #117e73;
}

h2,
h3 {
    color: #117e73;
}

table {
    width: 100%;
    border-collapse: collapse;
    background-color: white;
    margin-bottom: 30px;
}

th,
td {
    border: 1px solid #cccccc;
    padding: 10px;
    text-align: center;
}

th {
    background-color: #117e73;
    color: white;
}

tr:nth-child(even) {
    background-color: #f0f8f6;
}

tr:hover {
    background-color: #dcefeb;
}

.stats {
    background-color: white;
    padding: 20px;
    border-top: 5px solid #117e73;
    border-radius: 8px;
    margin-bottom: 30px;
}

.stats-grid {
    display: grid;
    grid-template-columns:
        repeat(
            auto-fit,
            minmax(220px, 1fr)
        );
    gap: 15px;
}

.stat-box {
    background-color: #f8f8f8;
    padding: 15px;
    border: 1px solid #dddddd;
    border-radius: 6px;
}

a {
    display: inline-block;
    margin-top: 15px;
    padding: 10px 15px;
    background-color: #117e73;
    color: white;
    text-decoration: none;
    border-radius: 5px;
}

a:hover {
    background-color: #0e665d;
}

</style>

</head>

<body>

<h1>
EE 470 IoT Sensor Database
</h1>


<h2>
Registered Sensors
</h2>

<table>

<tr>
    <th>Node Name</th>
    <th>Manufacturer</th>
    <th>Longitude</th>
    <th>Latitude</th>
</tr>

<?php while ($row = $resultRegister->fetch_assoc()) { ?>

<tr>

<td>
<?php echo htmlspecialchars($row["node_name"]); ?>
</td>

<td>
<?php echo htmlspecialchars($row["manufacturer"]); ?>
</td>

<td>
<?php echo $row["longitude"]; ?>
</td>

<td>
<?php echo $row["latitude"]; ?>
</td>

</tr>

<?php } ?>

</table>


<h2>
Sensor Measurements
</h2>

<table>

<tr>
    <th>Node</th>
    <th>Time Received</th>
    <th>Temperature (&deg;C)</th>
    <th>Humidity (%)</th>
    <th>Light Voltage (V)</th>
    <th>Distance (cm)</th>
</tr>

<?php while ($row = $resultData->fetch_assoc()) { ?>

<tr>

<td>
<?php echo htmlspecialchars($row["node_name"]); ?>
</td>

<td>
<?php echo htmlspecialchars($row["time_received"]); ?>
</td>

<td>
<?php
echo $row["temperature"] !== null
    ? number_format($row["temperature"], 2)
    : "--";
?>
</td>

<td>
<?php
echo $row["humidity"] !== null
    ? number_format($row["humidity"], 2)
    : "--";
?>
</td>

<td>
<?php
echo $row["light_voltage"] !== null
    ? number_format($row["light_voltage"], 3)
    : "--";
?>
</td>

<td>
<?php
echo $row["distance_cm"] !== null
    ? number_format($row["distance_cm"], 2)
    : "--";
?>
</td>

</tr>

<?php } ?>

</table>


<h2>
Sensor Activity
</h2>

<table>

<tr>
    <th>Node</th>
    <th>Valid Messages Received</th>
</tr>

<?php while ($row = $resultActivity->fetch_assoc()) { ?>

<tr>

<td>
<?php echo htmlspecialchars($row["node_name"]); ?>
</td>

<td>
<?php echo $row["message_count"]; ?>
</td>

</tr>

<?php } ?>

</table>


<h2>
Node 1 Statistics
</h2>

<div class="stats">

<div class="stats-grid">


<div class="stat-box">

<h3>
Temperature
</h3>

<p>
Average:
<strong>
<?php
echo $stats["avg_temperature"] !== null
    ? number_format($stats["avg_temperature"], 2)
    : "--";
?>
&deg;C
</strong>
</p>

<p>
Minimum:
<strong>
<?php
echo $stats["min_temperature"] !== null
    ? number_format($stats["min_temperature"], 2)
    : "--";
?>
&deg;C
</strong>
</p>

<p>
Maximum:
<strong>
<?php
echo $stats["max_temperature"] !== null
    ? number_format($stats["max_temperature"], 2)
    : "--";
?>
&deg;C
</strong>
</p>

</div>


<div class="stat-box">

<h3>
Humidity
</h3>

<p>
Average:
<strong>
<?php
echo $stats["avg_humidity"] !== null
    ? number_format($stats["avg_humidity"], 2)
    : "--";
?>
%
</strong>
</p>

<p>
Minimum:
<strong>
<?php
echo $stats["min_humidity"] !== null
    ? number_format($stats["min_humidity"], 2)
    : "--";
?>
%
</strong>
</p>

<p>
Maximum:
<strong>
<?php
echo $stats["max_humidity"] !== null
    ? number_format($stats["max_humidity"], 2)
    : "--";
?>
%
</strong>
</p>

</div>


<div class="stat-box">

<h3>
Light Voltage
</h3>

<p>
Average:
<strong>
<?php
echo $stats["avg_light"] !== null
    ? number_format($stats["avg_light"], 3)
    : "--";
?>
V
</strong>
</p>

<p>
Minimum:
<strong>
<?php
echo $stats["min_light"] !== null
    ? number_format($stats["min_light"], 3)
    : "--";
?>
V
</strong>
</p>

<p>
Maximum:
<strong>
<?php
echo $stats["max_light"] !== null
    ? number_format($stats["max_light"], 3)
    : "--";
?>
V
</strong>
</p>

</div>


<div class="stat-box">

<h3>
Distance
</h3>

<p>
Average:
<strong>
<?php
echo $stats["avg_distance"] !== null
    ? number_format($stats["avg_distance"], 2)
    : "--";
?>
cm
</strong>
</p>

<p>
Minimum:
<strong>
<?php
echo $stats["min_distance"] !== null
    ? number_format($stats["min_distance"], 2)
    : "--";
?>
cm
</strong>
</p>

<p>
Maximum:
<strong>
<?php
echo $stats["max_distance"] !== null
    ? number_format($stats["max_distance"], 2)
    : "--";
?>
cm
</strong>
</p>

</div>

</div>

</div>


<a href="Chartjs/gragh.html">
View Interactive Sensor Graph
</a>

</body>

</html>

<?php

$conn->close();

?>