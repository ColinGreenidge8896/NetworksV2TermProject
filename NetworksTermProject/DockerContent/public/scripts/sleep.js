// This script handles the sleep functionality of the robot.
function putRobotToSleep() {
    fetch("/telecommand", {
        // This method is using PUT to send the command to the robot.
        method: "PUT",
        headers: { "Content-Type": "application/json" },
        // Parse the command to JSON format.
        body: JSON.stringify({ command: "sleep" })
    })
        .then(res => res.text())
        .then(msg => {
            document.getElementById("sleepResult").textContent = msg;
        })
        .catch(err => {
            document.getElementById("sleepResult").textContent = "Error: " + err;
        });
}
