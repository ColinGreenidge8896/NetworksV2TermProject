// public/scripts/connect.js
function connectToRobot() {
    const ip = document.getElementById("robotIP").value;
    const port = document.getElementById("robotPort").value;
    const protocol = document.getElementById("protocol").value;

    fetch(`/connect/${ip}/${port}`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ protocol: protocol })  // send TCP/UDP in body
    })
        .then(response => response.text())
        .then(msg => {
            document.getElementById("connectResult").textContent = msg;
        })
        .catch(err => {
            document.getElementById("connectResult").textContent = "Error: " + err;
        });

    return false;
}