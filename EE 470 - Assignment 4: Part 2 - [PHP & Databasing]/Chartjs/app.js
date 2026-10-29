$(document).ready(function() {

    let chart;

    function loadSensor(node) {

        $.ajax({

            url: "data.php?node=" + encodeURIComponent(node),
            method: "GET",

            success: function(response) {

                console.log(response);

                var data = response.sensorData;
                var stats = response.stats;

                var time = [];
                var temperature = [];

                for (var i = 0; i < data.length; i++) {

                    time.push(data[i].time_received);

                    temperature.push(
                        parseFloat(data[i].temperature)
                    );
                }

                var chartdata = {

                    labels: time,

                    datasets: [
                        {
                            label: "Temperature",
                            backgroundColor: "rgba(0, 128, 0, 0.5)",
                            borderColor: "rgba(0, 128, 0, 1)",
                            borderWidth: 1,
                            data: temperature
                        }
                    ]
                };


                // Destroy old chart before creating new one
                if (chart) {
                    chart.destroy();
                }


                var ctx = document
                    .getElementById("mycanvas")
                    .getContext("2d");


                chart = new Chart(ctx, {

                    type: "bar",

                    data: chartdata,

                    options: {

                        responsive: true,

                        plugins: {

                            title: {
                                display: true,
                                text: node
                            }
                        },

                        scales: {

                            x: {
                                title: {
                                    display: true,
                                    text: "Time"
                                }
                            },

                            y: {
                                title: {
                                    display: true,
                                    text: "Temperature"
                                },

                                beginAtZero: false
                            }
                        }
                    }
                });


                // Update page title
                $("#pageTitle").text(
                    node + " Temperature"
                );

                $("#statsTitle").text(
                    node + " Statistics"
                );


                // Update statistics
                $("#avgTemp").text(
                    parseFloat(
                        stats.avg_temperature
                    ).toFixed(2)
                );

                $("#avgHumidity").text(
                    parseFloat(
                        stats.avg_humidity
                    ).toFixed(2)
                );

                $("#maxTemp").text(
                    parseFloat(
                        stats.max_temperature
                    ).toFixed(2)
                );

                $("#minHumidity").text(
                    parseFloat(
                        stats.min_humidity
                    ).toFixed(2)
                );
            },

            error: function(xhr, status, error) {

                console.log("AJAX error:");
                console.log(error);
                console.log(xhr.responseText);
            }
        });
    }


    // Load node_1 first
    loadSensor("node_1");


    // Run when dropdown changes
    $("#nodeSelect").change(function() {

        var selectedNode = $(this).val();

        loadSensor(selectedNode);

    });

});