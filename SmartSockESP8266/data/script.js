function toggleSwitch() {
    fetch("toggle");
}

function resetSwitch() {
    if (confirm("Are you sure to reset the energy usage meter?"))
        fetch("reset");
}

var isDataReceiving = false;
function fetchData() {
    if (!isDataReceiving) {
        isDataReceiving = true;
        fetch("data").then(function(response) {
            response.text().then(function(text){
                isDataReceiving = false;
                updateData(text);
            });
        });
    }
}

function updateData(data) {
    data = data.split(",");
    if (data.length == 6)
    {
        document.getElementById("txtVolts").innerHTML = data[0];
        document.getElementById("txtHz").innerHTML = data[1];
        document.getElementById("txtWatts").innerHTML = data[2];
        document.getElementById("txtKWh").innerHTML = data[3];
        document.getElementById("txtPower").innerHTML = data[4];
        if (data[5] == 1) {
            document.getElementById("btnToggleSwitch").className =
                document.getElementById("btnToggleSwitch").className.replace
                    ( /(?:^|\s)button-off(?!\S)/g , '' );

            if ( !(document.getElementById("btnToggleSwitch").className.match(/(?:^|\s)button-on(?!\S)/)) ) {
                document.getElementById("btnToggleSwitch").className += " button-on";
            }
        } else {
            document.getElementById("btnToggleSwitch").className =
                    document.getElementById("btnToggleSwitch").className.replace
                        ( /(?:^|\s)button-on(?!\S)/g , '' );

            if ( !(document.getElementById("btnToggleSwitch").className.match(/(?:^|\s)button-off(?!\S)/)) ) {
                document.getElementById("btnToggleSwitch").className += " button-off";
            }
        }
    }
}

setInterval(fetchData, 1000);