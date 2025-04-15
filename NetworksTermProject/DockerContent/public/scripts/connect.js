// public/scripts/connect.js
function connectToRobot() {
    const ip = document.getElementById("robotIP").value;
    const port = document.getElementById("robotPort").value;

    fetch(`/connect/${ip}/${port}`, { method: "POST" })
        .then(response => response.text())
        .then(data => {
            document.getElementById("connectResult").textContent = data;
        })
        .catch(err => {
            document.getElementById("connectResult").textContent = "Error: " + err;
        });

    return false;
}
