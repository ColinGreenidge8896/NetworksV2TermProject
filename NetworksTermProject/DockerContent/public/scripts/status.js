// public/scripts/status.js
// This script handles the fetching of telemetry data from the robot.
function fetchStatus() {
    fetch("/telemetry_request/")
        .then(response => response.text())
        .then(data => {
            document.getElementById("telemetryOutput").value = data;
        })
        .catch(err => {
            document.getElementById("telemetryOutput").value = "Error fetching telemetry.\n" + err;
        });
}
