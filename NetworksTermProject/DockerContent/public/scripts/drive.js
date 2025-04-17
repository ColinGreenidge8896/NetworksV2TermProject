function sendDrive() {
    const direction = document.getElementById("direction").value;
    const duration = document.getElementById("duration").value;
    const speed = document.getElementById("speed").value;

    fetch("/telecommand", {
        method: "PUT",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
            command: "drive",
            direction: parseInt(direction),
            duration: parseInt(duration),
            speed: parseInt(speed)
        })
    })
        .then(res => res.text())
        .then(msg => {
            document.getElementById("driveResult").textContent = msg;
        })
        .catch(err => {
            document.getElementById("driveResult").textContent = "Error: " + err;
        });

    return false;
}
