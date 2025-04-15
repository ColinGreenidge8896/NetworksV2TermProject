// public/scripts/drive.js
function sendDrive() {
    const direction = parseInt(document.getElementById("direction").value);
    const duration = parseInt(document.getElementById("duration").value);
    const speed = parseInt(document.getElementById("speed").value);

    const url = `/telecommand/${direction}/${duration}/${speed}`;

    fetch(url, {
        method: "PUT",
        headers: {
            'Content-Type': 'application/json'
        }
    })
    .then(response => response.text().then(text => ({ status: response.status, text })))
    .then(({ status, text }) => {
        if (status === 200) {
            document.getElementById("driveResult").textContent = "Success: " + text;
        } else {
            document.getElementById("driveResult").textContent = "Error: " + text;
        }
    })
    .catch(err => {
        document.getElementById("driveResult").textContent = "Error: " + err;
    });

    return false;
}