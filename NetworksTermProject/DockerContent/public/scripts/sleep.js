function putRobotToSleep() {
    fetch("/telecommand", {
        method: "PUT",
        headers: { "Content-Type": "application/json" },
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
