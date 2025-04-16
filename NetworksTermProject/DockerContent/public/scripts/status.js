// public/scripts/status.js
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
