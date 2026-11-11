let temperatureChart = null;
let humidityChart = null;
let lightChart = null;
let distanceChart = null;


// --------------------------------------------------
// Load selected node
// --------------------------------------------------

function loadSensor(node) {

    $.ajax({

        url:
            "data.php?node="
            + encodeURIComponent(node),

        method:
            "GET",

        dataType:
            "json",


        success:
            function(response) {

                console.log(response);


                const times = [];

                const temperatureValues = [];
                const humidityValues = [];
                const lightValues = [];
                const distanceValues = [];


                response.sensorData.forEach(
                    function(row) {

                        times.push(
                            row.time_received
                        );


                        temperatureValues.push(

                            row.temperature !== null

                            ? parseFloat(
                                row.temperature
                            )

                            : null
                        );


                        humidityValues.push(

                            row.humidity !== null

                            ? parseFloat(
                                row.humidity
                            )

                            : null
                        );


                        lightValues.push(

                            row.light_voltage !== null

                            ? parseFloat(
                                row.light_voltage
                            )

                            : null
                        );


                        distanceValues.push(

                            row.distance_cm !== null

                            ? parseFloat(
                                row.distance_cm
                            )

                            : null
                        );

                    }
                );


                // ----------------------------------
                // Destroy existing charts
                // ----------------------------------

                if (temperatureChart) {
                    temperatureChart.destroy();
                }

                if (humidityChart) {
                    humidityChart.destroy();
                }

                if (lightChart) {
                    lightChart.destroy();
                }

                if (distanceChart) {
                    distanceChart.destroy();
                }


                // ----------------------------------
                // Common green dataset settings
                // ----------------------------------

                const greenBorder =
                    "rgba(0, 128, 0, 1)";

                const greenBackground =
                    "rgba(0, 128, 0, 0.25)";


                // ----------------------------------
                // Temperature chart
                // ----------------------------------

                temperatureChart =
                    new Chart(

                        document
                        .getElementById(
                            "temperatureChart"
                        )
                        .getContext("2d"),

                        {

                            type: "line",

                            data: {

                                labels:
                                    times,

                                datasets: [

                                    {

                                        label:
                                            "Temperature (C)",

                                        data:
                                            temperatureValues,

                                        borderColor:
                                            greenBorder,

                                        backgroundColor:
                                            greenBackground,

                                        pointBackgroundColor:
                                            greenBorder,

                                        pointBorderColor:
                                            greenBorder,

                                        borderWidth:
                                            2,

                                        fill:
                                            false,

                                        spanGaps:
                                            true
                                    }

                                ]

                            },


                            options: {

                                responsive:
                                    true,

                                scales: {

                                    x: {

                                        title: {

                                            display:
                                                true,

                                            text:
                                                "Time"
                                        }

                                    },

                                    y: {

                                        title: {

                                            display:
                                                true,

                                            text:
                                                "Temperature (C)"
                                        }

                                    }

                                }

                            }

                        }
                    );


                // ----------------------------------
                // Humidity chart
                // ----------------------------------

                humidityChart =
                    new Chart(

                        document
                        .getElementById(
                            "humidityChart"
                        )
                        .getContext("2d"),

                        {

                            type:
                                "line",

                            data: {

                                labels:
                                    times,

                                datasets: [

                                    {

                                        label:
                                            "Humidity (%)",

                                        data:
                                            humidityValues,

                                        borderColor:
                                            greenBorder,

                                        backgroundColor:
                                            greenBackground,

                                        pointBackgroundColor:
                                            greenBorder,

                                        pointBorderColor:
                                            greenBorder,

                                        borderWidth:
                                            2,

                                        fill:
                                            false,

                                        spanGaps:
                                            true
                                    }

                                ]

                            },


                            options: {

                                responsive:
                                    true,

                                scales: {

                                    x: {

                                        title: {

                                            display:
                                                true,

                                            text:
                                                "Time"
                                        }

                                    },

                                    y: {

                                        title: {

                                            display:
                                                true,

                                            text:
                                                "Humidity (%)"
                                        }

                                    }

                                }

                            }

                        }
                    );


                // ----------------------------------
                // Light chart
                // ----------------------------------

                lightChart =
                    new Chart(

                        document
                        .getElementById(
                            "lightChart"
                        )
                        .getContext("2d"),

                        {

                            type:
                                "line",

                            data: {

                                labels:
                                    times,

                                datasets: [

                                    {

                                        label:
                                            "Light Voltage (V)",

                                        data:
                                            lightValues,

                                        borderColor:
                                            greenBorder,

                                        backgroundColor:
                                            greenBackground,

                                        pointBackgroundColor:
                                            greenBorder,

                                        pointBorderColor:
                                            greenBorder,

                                        borderWidth:
                                            2,

                                        fill:
                                            false,

                                        spanGaps:
                                            true
                                    }

                                ]

                            },


                            options: {

                                responsive:
                                    true,

                                scales: {

                                    x: {

                                        title: {

                                            display:
                                                true,

                                            text:
                                                "Time"
                                        }

                                    },

                                    y: {

                                        title: {

                                            display:
                                                true,

                                            text:
                                                "Voltage (V)"
                                        }

                                    }

                                }

                            }

                        }
                    );


                // ----------------------------------
                // Distance chart
                // ----------------------------------

                distanceChart =
                    new Chart(

                        document
                        .getElementById(
                            "distanceChart"
                        )
                        .getContext("2d"),

                        {

                            type:
                                "line",

                            data: {

                                labels:
                                    times,

                                datasets: [

                                    {

                                        label:
                                            "Distance (cm)",

                                        data:
                                            distanceValues,

                                        borderColor:
                                            greenBorder,

                                        backgroundColor:
                                            greenBackground,

                                        pointBackgroundColor:
                                            greenBorder,

                                        pointBorderColor:
                                            greenBorder,

                                        borderWidth:
                                            2,

                                        fill:
                                            false,

                                        spanGaps:
                                            true
                                    }

                                ]

                            },


                            options: {

                                responsive:
                                    true,

                                scales: {

                                    x: {

                                        title: {

                                            display:
                                                true,

                                            text:
                                                "Time"
                                        }

                                    },

                                    y: {

                                        title: {

                                            display:
                                                true,

                                            text:
                                                "Distance (cm)"
                                        }

                                    }

                                }

                            }

                        }
                    );


                // ----------------------------------
                // Page title
                // ----------------------------------

                const displayNode =
                    response.node
                    .replace(
                        "_",
                        " "
                    )
                    .toUpperCase();


                $("#pageTitle").text(
                    displayNode
                    + " Sensor Data"
                );


                $("#statsTitle").text(
                    displayNode
                    + " Statistics"
                );


                // ----------------------------------
                // Statistics
                // ----------------------------------

                const stats =
                    response.stats;


                $("#avgTemperature").text(

                    stats.avg_temperature !== null

                    ? parseFloat(
                        stats.avg_temperature
                    ).toFixed(2)

                    : "--"
                );


                $("#minTemperature").text(

                    stats.min_temperature !== null

                    ? parseFloat(
                        stats.min_temperature
                    ).toFixed(2)

                    : "--"
                );


                $("#maxTemperature").text(

                    stats.max_temperature !== null

                    ? parseFloat(
                        stats.max_temperature
                    ).toFixed(2)

                    : "--"
                );


                $("#avgHumidity").text(

                    stats.avg_humidity !== null

                    ? parseFloat(
                        stats.avg_humidity
                    ).toFixed(2)

                    : "--"
                );


                $("#minHumidity").text(

                    stats.min_humidity !== null

                    ? parseFloat(
                        stats.min_humidity
                    ).toFixed(2)

                    : "--"
                );


                $("#maxHumidity").text(

                    stats.max_humidity !== null

                    ? parseFloat(
                        stats.max_humidity
                    ).toFixed(2)

                    : "--"
                );


                $("#avgLight").text(

                    stats.avg_light !== null

                    ? parseFloat(
                        stats.avg_light
                    ).toFixed(3)

                    : "--"
                );


                $("#minLight").text(

                    stats.min_light !== null

                    ? parseFloat(
                        stats.min_light
                    ).toFixed(3)

                    : "--"
                );


                $("#maxLight").text(

                    stats.max_light !== null

                    ? parseFloat(
                        stats.max_light
                    ).toFixed(3)

                    : "--"
                );


                $("#avgDistance").text(

                    stats.avg_distance !== null

                    ? parseFloat(
                        stats.avg_distance
                    ).toFixed(2)

                    : "--"
                );


                $("#minDistance").text(

                    stats.min_distance !== null

                    ? parseFloat(
                        stats.min_distance
                    ).toFixed(2)

                    : "--"
                );


                $("#maxDistance").text(

                    stats.max_distance !== null

                    ? parseFloat(
                        stats.max_distance
                    ).toFixed(2)

                    : "--"
                );


                $("#messageCount").text(
                    response.messageCount
                );

            },


        error:
            function(xhr) {

                console.log(
                    xhr.responseText
                );

                alert(
                    "Unable to load sensor data."
                );
            }

    });
}


// --------------------------------------------------
// Dropdown
// --------------------------------------------------

$("#nodeSelect").on(
    "change",
    function() {

        loadSensor(
            $(this).val()
        );
    }
);


// --------------------------------------------------
// Initial load
// --------------------------------------------------

loadSensor(
    $("#nodeSelect").val()
);