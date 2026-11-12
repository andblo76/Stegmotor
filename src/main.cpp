#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// Add your Wi-Fi network details here. Leave both values empty to use the ESP32 access point.
const char* wifiSsid = "";
const char* wifiPassword = "";

const IPAddress apIp(192, 168, 5, 5);
const IPAddress apGateway(192, 168, 5, 5);
const IPAddress apSubnet(255, 255, 255, 0);

WebServer server(80);

constexpr uint8_t stepPin = 18;
constexpr uint8_t directionPin = 19;
constexpr uint8_t pwmChannel = 0;
constexpr uint8_t pwmResolution = 8;
constexpr uint16_t minSpeedPps = 50;
constexpr uint16_t maxSpeedPps = 5000;
constexpr uint16_t initialSpeedPps = 200;
constexpr uint16_t speedRampStepPps = 25;
constexpr unsigned long speedRampIntervalMs = 50;

bool motorOutputReady = false;
bool motorRunning = false;
uint8_t motorDirection = 0;
uint16_t targetSpeedPps = initialSpeedPps;
uint16_t currentSpeedPps = 0;
unsigned long lastRampUpdate = 0;

const char page[] PROGMEM = R"html(
<!doctype html>
<html lang="sv">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="theme-color" content="#080909">
  <title>Stegmotorstyrning</title>
  <style>
    :root {
      color-scheme: dark;
      font-family: Inter, ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
      background: #080909;
      color: #f2f4f2;
      font-synthesis: none;
      text-rendering: optimizeLegibility;
      -webkit-font-smoothing: antialiased;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      min-height: 100vh;
      background: radial-gradient(ellipse at 50% 0%, #111614 0, #080909 48rem);
    }
    main {
      width: min(100% - 40px, 880px);
      margin: 0 auto;
      padding: 34px 0 56px;
    }
    .topbar {
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding-bottom: 24px;
      border-bottom: 1px solid #252927;
    }
    .brand {
      display: flex;
      align-items: center;
      gap: 12px;
      color: #e8ebe8;
      font-size: 0.82rem;
      font-weight: 650;
      letter-spacing: 0.08em;
      text-transform: uppercase;
    }
    .brand-mark {
      display: grid;
      width: 34px;
      height: 34px;
      place-items: center;
      border: 1px solid #39433d;
      border-radius: 9px;
      color: #b7f36b;
      font-size: 0.76rem;
      letter-spacing: 0;
    }
    .version {
      color: #737b75;
      font-size: 0.72rem;
      letter-spacing: 0.12em;
    }
    .intro { padding: 58px 0 30px; }
    .eyebrow {
      margin: 0 0 13px;
      color: #a7d66c;
      font-size: 0.7rem;
      font-weight: 650;
      letter-spacing: 0.16em;
      text-transform: uppercase;
    }
    h1 {
      margin: 0;
      font-size: clamp(2.15rem, 7vw, 3.5rem);
      font-weight: 580;
      letter-spacing: -0.055em;
      line-height: 1.08;
    }
    .description {
      margin: 16px 0 0;
      color: #959d97;
      font-size: 1rem;
      line-height: 1.7;
    }
    .panel {
      overflow: hidden;
      border: 1px solid #2b312d;
      border-radius: 14px;
      background: linear-gradient(145deg, #111412, #0c0e0d 72%);
      box-shadow: 0 18px 60px #0005;
    }
    .panel-heading {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 16px;
      padding: 22px 24px;
      border-bottom: 1px solid #252a27;
    }
    .panel-label {
      margin: 0;
      color: #aeb5af;
      font-size: 0.76rem;
      font-weight: 600;
      letter-spacing: 0.1em;
      text-transform: uppercase;
    }
    .status {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      padding: 7px 10px;
      border: 1px solid #31412d;
      border-radius: 999px;
      background: #172016;
      color: #b7e998;
      font-size: 0.72rem;
      font-weight: 600;
      white-space: nowrap;
    }
    .status::before {
      width: 7px;
      height: 7px;
      border-radius: 50%;
      background: #9de36d;
      content: "";
      box-shadow: 0 0 10px #9de36d70;
    }
    .panel-body { padding: 25px 24px 27px; }
    .panel-body p {
      margin: 0;
      color: #e3e7e3;
      font-size: 1.05rem;
      line-height: 1.65;
    }
    .details {
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      gap: 14px;
      margin-top: 16px;
    }
    .detail {
      padding: 18px 20px;
      border: 1px solid #252a27;
      border-radius: 11px;
      background: #0c0e0d;
    }
    .detail-label {
      margin: 0 0 9px;
      color: #737c75;
      font-size: 0.68rem;
      font-weight: 600;
      letter-spacing: 0.12em;
      text-transform: uppercase;
    }
    .detail-value {
      margin: 0;
      color: #dfe4df;
      font-size: 0.92rem;
      font-weight: 550;
    }
    .status.stopped {
      border-color: #3b3d35;
      background: #1a1b17;
      color: #d3d1bf;
    }
    .status.stopped::before { background: #c4b878; box-shadow: 0 0 10px #c4b87860; }
    .status.offline {
      border-color: #503331;
      background: #211514;
      color: #f0aaa0;
    }
    .status.offline::before { background: #ef766a; box-shadow: 0 0 10px #ef766a60; }
    .speed-readout { display: flex; align-items: baseline; gap: 10px; margin-bottom: 8px; }
    .speed-value {
      color: #f2f3ee;
      font-size: clamp(2.8rem, 9vw, 4rem);
      font-variant-numeric: tabular-nums;
      font-weight: 580;
      letter-spacing: -0.07em;
      line-height: 1;
    }
    .speed-unit, .speed-current { color: #929a8f; font-size: 0.82rem; }
    .speed-current { margin: 0 0 25px; }
    input[type="range"] {
      width: 100%;
      height: 5px;
      margin: 8px 0 12px;
      border-radius: 999px;
      background: #6d8b50;
      cursor: pointer;
      accent-color: #b9ed78;
    }
    input[type="range"]:focus-visible, button:focus-visible {
      outline: 2px solid #b9ed78;
      outline-offset: 3px;
    }
    .range-labels { display: flex; justify-content: space-between; color: #717a70; font-size: 0.72rem; }
    .direction-options, .run-actions { display: grid; grid-template-columns: repeat(2, minmax(0, 1fr)); gap: 9px; }
    button {
      min-height: 44px;
      border: 1px solid #353d34;
      border-radius: 9px;
      background: #10130f;
      color: #aab1a5;
      font: inherit;
      font-size: 0.83rem;
      font-weight: 600;
      cursor: pointer;
    }
    button:hover:not(:disabled) { border-color: #71845b; color: #edf2e8; }
    button:disabled, input:disabled { cursor: not-allowed; opacity: 0.45; }
    .direction-button.selected { border-color: #718e50; background: #1a2415; color: #c5ed97; }
    .start-button { border-color: #a9da74; background: #b6e783; color: #11170d; }
    .start-button:hover:not(:disabled) { background: #c4f293; color: #11170d; }
    .stop-button { border-color: #4a3532; background: #211715; color: #e3aaa2; }
    .stop-button:hover:not(:disabled) { border-color: #89524a; color: #ffc0b7; }
    .detail-note { margin: 12px 0 0; color: #838b7f; font-size: 0.75rem; line-height: 1.55; }
    .message { min-height: 19px; margin: 12px 0 0; color: #a8b29e; font-size: 0.76rem; line-height: 1.5; }
    .message.error { color: #f0aaa0; }
    .notice {
      margin-top: 16px;
      padding: 15px 18px;
      border: 1px solid #292e28;
      border-radius: 11px;
      background: #0c0e0c;
      color: #90988d;
      font-size: 0.77rem;
      line-height: 1.65;
    }
    .notice strong { color: #c1c7bb; font-weight: 600; }
    footer {
      margin-top: 30px;
      color: #606862;
      font-size: 0.75rem;
      text-align: center;
    }
    @media (max-width: 520px) {
      main { width: min(100% - 28px, 880px); padding-top: 22px; }
      .intro { padding-top: 44px; }
      .panel-heading { align-items: flex-start; flex-direction: column; padding: 19px; }
      .panel-body { padding: 20px 19px; }
      .details { grid-template-columns: 1fr; gap: 10px; }
      .detail { padding: 16px 18px; }
    }
  </style>
</head>
<body>
  <main>
    <header class="topbar">
      <div class="brand"><span class="brand-mark" aria-hidden="true">SM</span> Stegmotor</div>
      <span class="version">ESP32 &middot; MOTORPANEL</span>
    </header>
    <section class="intro" aria-labelledby="page-title">
      <p class="eyebrow">Rörelsestyrning</p>
      <h1 id="page-title">Stegmotorstyrning</h1>
      <p class="description">Justera motorvarvtalet och styr motorns rörelse.</p>
    </section>
    <section class="panel" aria-label="Motorstyrning">
      <div class="panel-heading">
        <h2 class="panel-label">Motorpanel</h2>
        <span id="status" class="status stopped" role="status">Ansluter ...</span>
      </div>
      <div class="panel-body">
        <div class="speed-readout">
          <output id="speed-value" class="speed-value" for="speed">30</output>
          <span class="speed-unit">varv / min</span>
        </div>
        <p id="current-speed" class="speed-current">Aktuellt varvtal: 0 varv/min</p>
        <input id="speed" type="range" min="7.5" max="750" step="0.5" value="30" aria-label="Målhastighet i varv per minut">
        <div class="range-labels"><span>7,5</span><span>750 varv/min</span></div>
        <p id="speed-message" class="message" aria-live="polite"></p>
      </div>
    </section>
    <section class="details" aria-label="Riktning och körning">
      <article class="detail">
        <p class="detail-label">Riktning</p>
        <div class="direction-options">
          <button id="direction-a" class="direction-button selected" type="button" aria-pressed="true">Framåt</button>
          <button id="direction-b" class="direction-button" type="button" aria-pressed="false">Bakåt</button>
        </div>
        <p class="detail-note">Byt riktning när motorn står stilla. Det faktiska hållet beror på motorns inkoppling.</p>
      </article>
      <article class="detail">
        <p class="detail-label">Körning</p>
        <div class="run-actions">
          <button id="start" class="start-button" type="button" disabled>Starta</button>
          <button id="stop" class="stop-button" type="button" disabled>Stoppa</button>
        </div>
        <p id="message" class="message" role="status" aria-live="polite"></p>
      </article>
    </section>
    <aside class="notice"><strong>Börja försiktigt.</strong> Varvtalet visas enligt din angivna omräkning. Om DM542:ans mikrosteg ändras kan visat varvtal bli fel. Hastigheten rampas mjukt upp och vid ändring. Stoppet bryter inte motorns hållström.</aside>
    <footer>Stegmotorstyrning <span aria-hidden="true">&middot;</span> Lokal enhet</footer>
  </main>
  <script>
    const statusBadge = document.getElementById("status");
    const speedSlider = document.getElementById("speed");
    const speedValue = document.getElementById("speed-value");
    const currentSpeed = document.getElementById("current-speed");
    const directionA = document.getElementById("direction-a");
    const directionB = document.getElementById("direction-b");
    const startButton = document.getElementById("start");
    const stopButton = document.getElementById("stop");
    const message = document.getElementById("message");
    const speedMessage = document.getElementById("speed-message");
    const rpmPerPulsePerSecond = 0.15;
    let motorState = null;
    let requestInProgress = false;
    let sliderChanging = false;

    function formatRpm(pulsesPerSecond) {
      return (pulsesPerSecond * rpmPerPulsePerSecond).toLocaleString("sv-SE", { maximumFractionDigits: 1 });
    }

    function renderState(state) {
      motorState = state;
      if (!sliderChanging) {
        speedSlider.value = (state.speed * rpmPerPulsePerSecond).toFixed(1);
        speedValue.value = formatRpm(state.speed);
      }
      currentSpeed.textContent = "Aktuellt varvtal: " + formatRpm(state.currentSpeed) + " varv/min";
      directionA.classList.toggle("selected", state.direction === 0);
      directionB.classList.toggle("selected", state.direction === 1);
      directionA.setAttribute("aria-pressed", String(state.direction === 0));
      directionB.setAttribute("aria-pressed", String(state.direction === 1));
      statusBadge.classList.toggle("offline", !state.ready);
      statusBadge.classList.toggle("stopped", state.ready && !state.running);
      statusBadge.textContent = !state.ready ? "Motorutgång ej tillgänglig" : (state.running ? "Motorn kör" : "Motorn står stilla");
      startButton.disabled = requestInProgress || !state.ready || state.running;
      stopButton.disabled = requestInProgress || !state.running;
      directionA.disabled = requestInProgress || state.running || !state.ready;
      directionB.disabled = requestInProgress || state.running || !state.ready;
      speedSlider.disabled = requestInProgress || !state.ready;
    }

    async function refreshStatus() {
      try {
        const response = await fetch("/api/status", { cache: "no-store" });
        if (!response.ok) throw new Error("Status kunde inte hämtas.");
        renderState(await response.json());
      } catch (error) {
        motorState = null;
        statusBadge.className = "status offline";
        statusBadge.textContent = "Ingen kontakt med styrenheten";
        startButton.disabled = true;
        stopButton.disabled = true;
        directionA.disabled = true;
        directionB.disabled = true;
        speedSlider.disabled = true;
      }
    }

    async function sendCommand(action, value) {
      if (requestInProgress) return;
      requestInProgress = true;
      if (action === "speed") sliderChanging = true;
      if (motorState) renderState(motorState);
      const body = new URLSearchParams({ action });
      if (value !== undefined) body.set("value", value);
      try {
        const response = await fetch("/api/control", {
          method: "POST",
          headers: { "Content-Type": "application/x-www-form-urlencoded", "X-Motor-Command": "1" },
          body
        });
        const result = await response.json();
        if (!response.ok) throw new Error(result.error || "Kommandot kunde inte utföras.");
        message.textContent = "";
        message.classList.remove("error");
        speedMessage.textContent = action === "speed" && result.running ? "Hastigheten ändras mjukt." : "";
        await refreshStatus();
      } catch (error) {
        message.textContent = error.message || "Anslutningen misslyckades.";
        message.classList.add("error");
      } finally {
        requestInProgress = false;
        if (action === "speed") sliderChanging = false;
        if (motorState) renderState(motorState);
      }
    }

    speedSlider.addEventListener("pointerdown", () => { sliderChanging = true; });
    speedSlider.addEventListener("pointerup", () => { sliderChanging = false; });
    speedSlider.addEventListener("pointercancel", () => { sliderChanging = false; });
    speedSlider.addEventListener("keydown", () => { sliderChanging = true; });
    speedSlider.addEventListener("keyup", () => { sliderChanging = false; });
    speedSlider.addEventListener("input", () => {
      speedValue.value = Number(speedSlider.value).toLocaleString("sv-SE", { maximumFractionDigits: 1 });
    });
    speedSlider.addEventListener("change", () => {
      sendCommand("speed", Math.round(Number(speedSlider.value) / rpmPerPulsePerSecond));
    });
    directionA.addEventListener("click", () => sendCommand("direction", "0"));
    directionB.addEventListener("click", () => sendCommand("direction", "1"));
    startButton.addEventListener("click", () => sendCommand("start"));
    stopButton.addEventListener("click", () => sendCommand("stop"));

    refreshStatus();
    window.setInterval(refreshStatus, 1500);
  </script>
</body>
</html>
)html";

void sendMotorError(int statusCode, const char* error)
{
    char response[128];
    snprintf(response, sizeof(response), "{\"error\":\"%s\"}", error);
    server.send(statusCode, "application/json; charset=utf-8", response);
}

void sendMotorStatus()
{
    char response[160];
    snprintf(response, sizeof(response),
             "{\"ready\":%s,\"running\":%s,\"speed\":%u,\"currentSpeed\":%u,\"direction\":%u}",
             motorOutputReady ? "true" : "false",
             motorRunning ? "true" : "false",
             targetSpeedPps,
             currentSpeedPps,
             motorDirection);
    server.send(200, "application/json; charset=utf-8", response);
}

bool parseControlValue(long& value)
{
    if (!server.hasArg("value")) {
        return false;
    }

    const String argument = server.arg("value");
    char* end = nullptr;
    value = strtol(argument.c_str(), &end, 10);
    return end != argument.c_str() && *end == '\0';
}

void stopMotor()
{
    if (motorOutputReady) {
        ledcWrite(pwmChannel, 0);
    }
    motorRunning = false;
    currentSpeedPps = 0;
}

void startMotor()
{
    currentSpeedPps = minSpeedPps;
    if (ledcChangeFrequency(pwmChannel, currentSpeedPps, pwmResolution) == 0) {
        stopMotor();
        motorOutputReady = false;
        return;
    }

    digitalWrite(directionPin, motorDirection == 0 ? LOW : HIGH);
    delayMicroseconds(5);
    ledcWrite(pwmChannel, 128);
    motorRunning = true;
    lastRampUpdate = millis();
}

void handleMotorControl()
{
    if (server.header("X-Motor-Command") != "1") {
        sendMotorError(403, "Ogiltig kontrollbegäran.");
        return;
    }

    if (!server.hasArg("action")) {
        sendMotorError(400, "Kommandotyp saknas.");
        return;
    }

    const String action = server.arg("action");
    if (action == "stop") {
        stopMotor();
    } else if (action == "start") {
        if (!motorOutputReady) {
            sendMotorError(503, "Motorutgången är inte tillgänglig.");
            return;
        }
        if (!motorRunning) {
            startMotor();
        }
        if (!motorOutputReady) {
            sendMotorError(503, "Kunde inte ställa in motorpulser.");
            return;
        }
    } else if (action == "speed") {
        long requestedSpeed = 0;
        if (!parseControlValue(requestedSpeed) ||
            requestedSpeed < minSpeedPps || requestedSpeed > maxSpeedPps) {
            sendMotorError(400, "Hastigheten måste vara mellan 50 och 5000 pulser per sekund.");
            return;
        }
        targetSpeedPps = static_cast<uint16_t>(requestedSpeed);
    } else if (action == "direction") {
        long requestedDirection = -1;
        if (!parseControlValue(requestedDirection) ||
            (requestedDirection != 0 && requestedDirection != 1)) {
            sendMotorError(400, "Riktningen måste vara A eller B.");
            return;
        }
        if (motorRunning) {
            sendMotorError(409, "Stoppa motorn innan du byter riktning.");
            return;
        }
        motorDirection = static_cast<uint8_t>(requestedDirection);
        digitalWrite(directionPin, motorDirection == 0 ? LOW : HIGH);
        delayMicroseconds(5);
    } else {
        sendMotorError(400, "Okänt motorkommando.");
        return;
    }

    sendMotorStatus();
}

void updateMotorSpeed()
{
    if (!motorRunning || millis() - lastRampUpdate < speedRampIntervalMs) {
        return;
    }
    lastRampUpdate = millis();

    uint16_t nextSpeed = currentSpeedPps;
    if (currentSpeedPps < targetSpeedPps) {
        const uint16_t difference = targetSpeedPps - currentSpeedPps;
        nextSpeed += difference < speedRampStepPps ? difference : speedRampStepPps;
    } else if (currentSpeedPps > targetSpeedPps) {
        const uint16_t difference = currentSpeedPps - targetSpeedPps;
        nextSpeed -= difference < speedRampStepPps ? difference : speedRampStepPps;
    }

    if (nextSpeed != currentSpeedPps) {
        if (ledcChangeFrequency(pwmChannel, nextSpeed, pwmResolution) == 0) {
            Serial.println("Kunde inte ändra motorpulsernas frekvens.");
            stopMotor();
            motorOutputReady = false;
            return;
        }
        currentSpeedPps = nextSpeed;
    }
}

void handleNotFound()
{
    server.send(404, "text/plain; charset=utf-8", "Sidan hittades inte");
}

void setup()
{
    Serial.begin(115200);

    pinMode(stepPin, OUTPUT);
    digitalWrite(stepPin, LOW);
    pinMode(directionPin, OUTPUT);
    digitalWrite(directionPin, LOW);
    motorOutputReady = ledcSetup(pwmChannel, minSpeedPps, pwmResolution) != 0;
    if (motorOutputReady) {
        ledcAttachPin(stepPin, pwmChannel);
        ledcWrite(pwmChannel, 0);
    } else {
        Serial.println("Kunde inte konfigurera motorutgången.");
    }

    if (wifiSsid[0] != '\0') {
        WiFi.mode(WIFI_STA);
        WiFi.begin(wifiSsid, wifiPassword);

        Serial.print("Ansluter till Wi-Fi");
        const unsigned long timeout = millis() + 15000;
        while (WiFi.status() != WL_CONNECTED && millis() < timeout) {
            delay(250);
            Serial.print(".");
        }
    }

    if (WiFi.status() != WL_CONNECTED) {
        WiFi.mode(WIFI_AP);
        if (!WiFi.softAPConfig(apIp, apGateway, apSubnet)) {
            Serial.println("\nKunde inte konfigurera ESP32-accesspunktens IP-adress.");
            return;
        }
        if (!WiFi.softAP("Stegmotorstyrning")) {
            Serial.println("\nKunde inte starta ESP32-accesspunkten.");
            return;
        }
        Serial.println("\nAnslut till Wi-Fi-nätverket Stegmotorstyrning.");
        Serial.print("Öppna http://");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("\nAnsluten till Wi-Fi.");
        Serial.print("Öppna http://");
        Serial.println(WiFi.localIP());
    }

    server.on("/", []() {
        server.send_P(200, "text/html; charset=utf-8", page);
    });
    server.on("/api/status", HTTP_GET, sendMotorStatus);
    const char* controlHeaders[] = {"X-Motor-Command"};
    server.collectHeaders(controlHeaders, 1);
    server.on("/api/control", HTTP_POST, handleMotorControl);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("Webbservern är igång.");
}

void loop()
{
    server.handleClient();
    updateMotorSpeed();
}
