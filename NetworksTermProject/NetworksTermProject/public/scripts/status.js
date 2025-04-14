// public/scripts/status.js
function fetchStatus() {
    fetch("/status")
        .then(response => response.text())
        .then(data => {
            document.getElementById("output").textContent = data;
        })
        .catch(err => {
            document.getElementById("output").textContent = "Error fetching telemetry.\n" + err;
        });
}
