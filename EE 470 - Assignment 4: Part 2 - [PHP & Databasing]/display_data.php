<?php

// Database connection information
$hostname = "localhost";
$username = "u863989605_db_AntonioKass";
$password = "Blastwing472";
$database = "u863989605_AntonioKassis";

// Connect to database
$conn = new mysqli($hostname, $username, $password, $database);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Get registered sensor nodes
$sqlRegister = "
    SELECT node_name, manufacturer, longitude, latitude
    FROM sensor_register
    ORDER BY node_name
";

$resultRegister = $conn->query($sqlRegister);

if (!$resultRegister) {
    die("sensor_register query failed: " . $conn->error);
}

// Get sensor data
$sqlData = "
    SELECT node_name, time_received, temperature, humidity
    FROM sensor_data
    ORDER BY node_name, time_received
";

$resultData = $conn->query($sqlData);

if (!$resultData) {
    die("sensor_data query failed: " . $conn->error);
}

// Get averages for node_1
$sqlAverage = "
    SELECT
        AVG(temperature) AS avg_temperature,
        AVG(humidity) AS avg_humidity
    FROM sensor_data
    WHERE node_name = 'node_1'
";

$resultAverage = $conn->query($sqlAverage);

if (!$resultAverage) {
    die("Average query failed: " . $conn->error);
}

$averageRow = $resultAverage->fetch_assoc();

$avgTemperature = $averageRow["avg_temperature"];
$avgHumidity = $averageRow["avg_humidity"];

?>

<!DOCTYPE html>
<html>

<head>

    <title>EE 470 Sensor Database</title>

    <style>

        body {
            font-family: Arial, sans-serif;
            margin: 40px;
            background-color: #f5f5f5;
        }

        h1 {
            color: #117e73;
        }

        h2 {
            margin-top: 35px;
        }

        table {
            border-collapse: collapse;
            width: 850px;
            background-color: white;
        }

        th,
        td {
            border: 1px solid #999;
            padding: 8px;
            text-align: center;
        }

        th {
            background-color: #117e73;
            color: white;
        }

        tr:nth-child(even) {
            background-color: #eeeeee;
        }

    </style>

</head>

<body>

<h1>Welcome to Antonio's EE 470 IoT Lab</h1>


<h2>Registered Sensor Nodes</h2>

<table>

    <tr>
        <th>Node Name</th>
        <th>Manufacturer</th>
        <th>Longitude</th>
        <th>Latitude</th>
    </tr>

    <?php

    if ($resultRegister->num_rows > 0) {

        while ($row = $resultRegister->fetch_assoc()) {

            echo "<tr>";

            echo "<td>" . $row["node_name"] . "</td>";
            echo "<td>" . $row["manufacturer"] . "</td>";
            echo "<td>" . $row["longitude"] . "</td>";
            echo "<td>" . $row["latitude"] . "</td>";

            echo "</tr>";
        }

    } else {

        echo "<tr>";
        echo "<td colspan='4'>No registered sensors found</td>";
        echo "</tr>";
    }

    ?>

</table>



<h2>Sensor Data</h2>

<table>

    <tr>
        <th>Node Name</th>
        <th>Time Received</th>
        <th>Temperature</th>
        <th>Humidity</th>
    </tr>

    <?php

    if ($resultData->num_rows > 0) {

        while ($row = $resultData->fetch_assoc()) {

            echo "<tr>";

            echo "<td>" . $row["node_name"] . "</td>";
            echo "<td>" . $row["time_received"] . "</td>";
            echo "<td>" . $row["temperature"] . "</td>";
            echo "<td>" . $row["humidity"] . "</td>";

            echo "</tr>";
        }

    } else {

        echo "<tr>";
        echo "<td colspan='4'>No sensor data found</td>";
        echo "</tr>";
    }

    ?>

</table>

<h2>Node 1 Averages</h2>

<p>
    Average Temperature:
    <?php echo round($avgTemperature, 2); ?> °C
</p>

<p>
    Average Humidity:
    <?php echo round($avgHumidity, 2); ?> %
</p>

</body>

</html>

<?php

$conn->close();

?>