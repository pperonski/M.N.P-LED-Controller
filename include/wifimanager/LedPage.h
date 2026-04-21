#pragma once

const char ledPageHTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>DJ LED</title>
    <style>
        * {
            font-family: Arial, Helvetica, sans-serif;
        }

        body {
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 25px;
        }

        button {
            border: black solid 2px;
            background-color: crimson;
            padding: 10px;
            font-size: medium;
        }

        button:hover {
            background-color: black;
            color: white;
            cursor: pointer;
        }

        button:active {
            color: black;
        }

        .duty_slider {
            appearance: slider-vertical;
            height: fit-content;
        }

        .freq_slider {
            width: fit-content;
            height: fit-content;
        }

        main {
            display: flex;
            flex-direction: row;
            align-items: center;
            flex-wrap: wrap;
            width: 100%;
            height: fit-content;
            justify-content: center;
            gap: 10px;
        }

        .led_control {
            height: 50px;
            width: 100%;
            border-width: 3px;
            border-color: black;
            border-style: solid;
            background-color: rgb(226, 225, 225);

            animation-name: blink;
            animation-duration: 0;
            animation-iteration-count: infinite;
            animation-timing-function: step-start;
        }

        .led {
            display: flex;
            height: fit-content;
            justify-content: center;
            align-items: center;
            flex-direction: column;
            padding-left: 10px;
            padding-right: 10px;
            border-width: 1px;
            border-color: black;
            border-style: solid;
        }


        .led_control:hover {
            opacity: 0.6;
            color: white;
            cursor: pointer;
        }

        .led_active {
            border-color: red;
            border-width: 6px;
        }

        .led_clicked {
            background-color: rgb(255, 0, 0);
        }


        .led_control label,
        p {
            text-align: center;
        }

        footer {
            text-align: center;
            width: 100%;
        }

        @keyframes blink {
            0% {
                background-color: initial;
            }

            50% {
                background-color: transparent;
            }
        }

        @media screen and (max-width:600px) {
            footer {
                display: none;
            }
        }
    </style>
</head>

<body>
    <div>
        <button onclick="updateAllFreq(0.0)">Static</button>
        <button onclick="updateAllFreq(0.25)">0.25 Hz</button>
        <button onclick="updateAllFreq(0.5)">0.5 Hz</button>
        <button onclick="updateAllFreq(1.0)">1 Hz</button>
        <button onclick="updateAllFreq(10.0)">10 Hz</button>
        <button onclick="updateAllFreq(50.0)">50 Hz</button>
    </div>
    <main>

    </main>
    <footer>
        Use W and S to change Brightness, use P and L to change Frequency.
    </footer>
</body>
<script>

    const keysFunction = {
        '1': () => { toggleActive(0) },
        '2': () => { toggleActive(1) },
        '3': () => { toggleActive(2) },
        '4': () => { toggleActive(3) },
        '5': () => { toggleActive(4) },
        '6': () => { toggleActive(5) },
        '7': () => { toggleActive(6) },
        '8': () => { toggleActive(7) },
        '9': () => { toggleActive(8) },
        '0': () => { toggleActive(9) },
        '!': () => { toggleActive(10) },
        '@': () => { toggleActive(11) },
        '#': () => { toggleActive(12) },
        'w': () => { updateActiveDuty(0.1) },
        's': () => { updateActiveDuty(-0.1) },
        'p': () => { updateActiveFreq(0.1) },
        'l': () => { updateActiveFreq(-0.1) },
        'u': () => { toogleActiveLeds() }
    };

    function onKeyPress(event) {
        const key = event.key;
        console.log("Key pressed! key: " + key);

        if (keysFunction[key]) {
            keysFunction[key]();
        }
    }

    function updateActiveDuty(value) {
        const main = document.getElementsByTagName("main")[0];

        const active = [...main.getElementsByClassName("led_active")];

        active.forEach(element => {
            const id = Number.parseInt(element.id);

            const LedChannel = document.getElementById(id);

            const dutyValue = getDuty(id);

            updateDuty(id, Number.parseFloat(dutyValue) + Number.parseFloat(value));

        });
    }

    function toogleActiveLeds()
    {
        const main = document.getElementsByTagName("main")[0];

        const active = [...main.getElementsByClassName("led_active")];

        active.forEach(element => {
    
            onLedClick(id);

        });
    }

    function updateActiveFreq(value) {
        const main = document.getElementsByTagName("main")[0];

        const active = [...main.getElementsByClassName("led_active")];

        active.forEach(element => {
            const id = Number.parseInt(element.id);

            const LedChannel = document.getElementById(id);

            const freqValue = getFreq(id);

            updateFreq(id, Number.parseFloat(freqValue) + Number.parseFloat(value));

        });
    }

    function startUp() {
        const main = document.getElementsByTagName("main")[0];

        for (let i = 0; i < 13; i++) {
            main.innerHTML += `
            <div id="${i}" class="led" oncontextmenu="onLedSelect(this)">
            <div class="led_control" onclick="onLedClick(${i})">
            </div>
            <p>Channel: ${i}</p>
            <label class="duty_value">Brightness:</label><input class="duty_slider" id="duty_${i}" type="range" min="0.0" max="100.0" value="50.0" step="0.1" oninput="updateDuty(${i},this.value)">
            <label class="freq_value">Frequency:</label><input class="freq_slider" id="freq_${i}" type="range" min="0.0" max="10.0" value="0.0" step="0.1" oninput="updateFreq(${i},this.value)">
            </div>
            `;

        }

        updateAllDuty(50.0);
        updateAllFreq(0.0);
    }

    function onLedClick(id)
    {
        const LedChannel = document.getElementById(id);

        const ledButton = LedChannel.getElementsByClassName('led_control')[0];

        toggleLed(id,ledButton);
    }

    function toggleLed(id, item) {
        
        if(item.style.backgroundColor == "red")
        {
            item.style.backgroundColor = 'gray';
            sendDuty(id,0.0);
        }
        else
        {
            item.style.backgroundColor = 'red';
            const duty = getDuty(id);
            sendDuty(id,duty);
        }
    }

    function toggleActive(item_id) {
        const main_items = document.getElementsByTagName("main")[0];
        const items = main_items.getElementsByClassName("led");

        onLedSelect(items[item_id]);
    }

    function getDuty(item_id) {
        const dutySlider = document.getElementById(`duty_${item_id}`);

        return dutySlider?.value ?? 0;
    }

    function getFreq(item_id)
    {
        const freqSlider = document.getElementById(`freq_${item_id}`);

        return freqSlider?.value ?? 0;
    }

    function onLedSelect(item) {

        if (item.className.search("led_active") > -1) {
            item.className = item.className.replace(" led_active", "");
        }
        else {
            item.className += " led_active";
        }
    }

    function sendDuty(channel,value)
    {
        const request = new XMLHttpRequest();
        request.open("GET", `/led/duty/?id=${channel}&value=${value}`, false);
        request.send(null);
    }

    function sendFreq(channel,value)
    {
        const request = new XMLHttpRequest();
        request.open("GET", `/led/freq/?id=${channel}&value=${value}`, false);
        request.send(null);
    }

    function updateDuty(channel, value) {
        console.log("Duty ch: " + channel + " val: " + value);

        const LedChannel = document.getElementById(channel);

        const animateButton = LedChannel.getElementsByClassName('led_control')[0];

        const dutyValue = getDuty(channel);

        const dutyLabel = LedChannel.getElementsByClassName("duty_value")[0];

        dutyLabel.innerHTML = `Brigthness: ${dutyValue} %`;

        if (Number.parseFloat(dutyValue) != 0.0) {
            animateButton.style.backgroundColor = "red";
        }
        else {
            animateButton.style.backgroundColor = "gray";
        }

        sendDuty(channel,dutyValue);
    }

    function updateFreq(channel, value) {
        console.log("Freq ch: " + channel + " val: " + value);

        const element = document.getElementById(channel);

        const animateButton = element.getElementsByClassName('led_control')[0];

        const freqValue = getFreq(channel);

        const freqLabel = element.getElementsByClassName("freq_value")[0];

        freqLabel.innerHTML = `Frequency: ${freqValue} Hz`;

        if (Number.parseFloat(freqValue) != 0.0) {
            animateButton.style.animationDuration = `${1 / Number.parseFloat(freqValue)}s`;
            animateButton.style.animationName = "blink";
        }
        else {
            animateButton.style.animationName = "none";
        }

        sendFreq(channel,freqValue);
    }

    function updateAllDuty(value) {
        for (let i = 0; i < 13; i++) {
            updateDuty(i, value);
        }
    }

    function updateAllFreq(value) {
        for (let i = 0; i < 13; i++) {
            updateFreq(i, value);
        }
    }

    document.addEventListener("keydown", onKeyPress);

    startUp();

</script>
</html>
)rawliteral";