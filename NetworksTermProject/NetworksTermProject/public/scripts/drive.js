function sendDrive() {
    const direction = parseInt(document.getElementById("direction").value);
    const duration = parseInt(document.getElementById("duration").value);
    const speed = parseInt(document.getElementById("speed").value);

    const url = `/drive/${direction}/${duration}/${speed}`;
    fetch(url, { method: "POST" })
        .then(response => response.text())
        .then(data => {
            document.getElementById("driveResult").textContent = data;

            if (data.includes("acknowledged")) {
                alert("✅ Drive command acknowledged by simulator!");
            } else {
                alert("⚠️ Simulator responded, but not with DRIVE ACK.");
            }
        })
        .catch(error => {
            document.getElementById("driveResult").textContent = "❌ Error: " + error;
        });

    return false;
}
