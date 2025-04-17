//JavaScript script to put the robot to sleep
function putRobotToSleep() {
    fetch("/sleep", { method: "PUT" })
        .then(res => res.text())
        .then(msg => {
            document.getElementById("sleepResult").textContent = msg;
        })
        .catch(err => {
            document.getElementById("sleepResult").textContent = "Error: " + err;
        });
}
