function toggleSwitchMain() {
    fetch("toggleMain");
}

function toggleSwitch1() {
    fetch("toggle1");
}

function toggleSwitch2() {
    fetch("toggle2");
}

function toggleSwitch3() {
    fetch("toggle3");
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

function setButtonState(id, state) {
    if (state == 1) {
        document.getElementById(id).className =
            document.getElementById(id).className.replace
                ( /(?:^|\s)button-off(?!\S)/g , '' );

        if ( !(document.getElementById(id).className.match(/(?:^|\s)button-on(?!\S)/)) ) {
            document.getElementById(id).className += " button-on";
        }
    } else {
        document.getElementById(id).className =
                document.getElementById(id).className.replace
                    ( /(?:^|\s)button-on(?!\S)/g , '' );

        if ( !(document.getElementById(id).className.match(/(?:^|\s)button-off(?!\S)/)) ) {
            document.getElementById(id).className += " button-off";
        }
    }
}

function updateData(data) {
    data = data.split(",");
    if (data.length === 9)
    {
        document.getElementById("txtVolts").innerHTML = data[0];
        document.getElementById("txtHz").innerHTML = data[1];
        document.getElementById("txtWatts").innerHTML = data[2];
        document.getElementById("txtKWh").innerHTML = data[3];
        document.getElementById("txtPower").innerHTML = data[4];
        setButtonState("btnToggleSwitchMain", data[5]);
        setButtonState("btnToggleSwitch1", data[6]);
        setButtonState("btnToggleSwitch2", data[7]);
        setButtonState("btnToggleSwitch3", data[8]);
    }
}

setInterval(fetchData, 1000);