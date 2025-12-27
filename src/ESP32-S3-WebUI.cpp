/*
  Arduino code for ESP32-S3 (tested primarily on N16R8 version)
  Select: 4D Systems gen4-ESP32 16MB Modules (ESP32-S3R8n16) (esp32:esp32:gen4-ESP32-S3R8n16)
*/

#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

// ===== Wi-Fi AP =====
// access point http://192.168.4.1
const char* AP_SSID = "ESP32";
const char* AP_PASS = "testpass";

// ===== NeoPixel pins =====
#define LED_PIN    48
#define LED_COUNT  1

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Brightness 0..255 (keep modest)
static uint8_t BRIGHTNESS = 50;

// ===== Web Server =====
WebServer server(80); // Port 80

// Track current color for /status
static uint8_t curR = 0, curG = 0, curB = 0;

static void setColor(uint8_t r, uint8_t g, uint8_t b) {
  curR = r; curG = g; curB = b;
  strip.setPixelColor(0, strip.Color(r, g, b));
  strip.show();
}

static String rgbString() {
  char buf[48];
  snprintf(buf, sizeof(buf), "RGB(%u,%u,%u) Brightness=%u", curR, curG, curB, BRIGHTNESS);
  return String(buf);
}

// WebUI with CSS, HTML and JS
// TODO: Use LittleFS to have the HTML, CSS and JS in separate files
const char PAGE[] PROGMEM = R"HTML(
<!doctype html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1" charset="utf-8">
<title>ESP32 NeoPixel Control</title>
<style>
  :root{
    --bg:#0b1220;
    --card:#101a2e;
    --muted:#9fb0d0;
    --text:#eaf0ff;
    --border:rgba(255,255,255,.10);
    --shadow:0 12px 32px rgba(0,0,0,.35);
    --r:14px;
  }
  *{box-sizing:border-box}
  body{
    margin:0;
    font-family: system-ui, -apple-system, Segoe UI, Roboto, Arial, sans-serif;
    background: radial-gradient(1200px 600px at 20% 0%, #162751 0%, transparent 60%),
                radial-gradient(900px 600px at 80% 20%, #2a1456 0%, transparent 55%),
                var(--bg);
    color:var(--text);
    min-height:100vh;
    padding:18px;
  }
  .wrap{
    max-width:720px;
    margin:0 auto;
    display:flex;
    flex-direction:column;
    gap:14px;
  }
  .header{
    display:flex;
    align-items:flex-start;
    justify-content:space-between;
    gap:12px;
  }
  h1{
    font-size:20px;
    margin:0;
    letter-spacing:.2px;
  }
  .sub{
    margin:6px 0 0;
    color:var(--muted);
    font-size:13px;
    line-height:1.35;
  }
  .badge{
    display:flex;
    gap:8px;
    align-items:center;
    padding:10px 12px;
    border:1px solid var(--border);
    border-radius:999px;
    background:rgba(255,255,255,.06);
    box-shadow: var(--shadow);
    white-space:nowrap;
    user-select:none;
  }

  .dot{
    width:10px;height:10px;border-radius:999px;
    background:#7c3aed;
    box-shadow:0 0 16px rgba(124,58,237,.75);
  }

  .card{
    background: linear-gradient(180deg, rgba(255,255,255,.06), rgba(255,255,255,.03));
    border:1px solid var(--border);
    border-radius:var(--r);
    box-shadow:var(--shadow);
    padding:14px;
  }
  .row{
    display:flex;
    gap:10px;
    flex-wrap:wrap;
    align-items:center;
    justify-content:space-between;
  }

  .grid{
    display:grid;
    grid-template-columns: repeat(2, minmax(0,1fr));
    gap:10px;
    margin-top:10px;
  }
  @media (min-width:520px){
    .grid{ grid-template-columns: repeat(4, minmax(0,1fr)); }
  }

  button{
    width:100%;
    padding:12px 12px;
    border-radius:12px;
    border:1px solid var(--border);
    background:rgba(255,255,255,.06);
    color:var(--text);
    font-size:14px;
    font-weight:600;
    letter-spacing:.2px;
    cursor:pointer;
    transition: transform .05s ease, filter .15s ease, background .15s ease;
  }
  button:hover{ filter:brightness(1.08); }
  button:active{ transform: translateY(1px) scale(.99); }
  button:disabled{ opacity:.55; cursor:not-allowed; }

  .btn-off{ background:rgba(255,255,255,.05); }
  .btn-red{ background:rgba(255,60,60,.18); border-color:rgba(255,60,60,.28); }
  .btn-green{ background:rgba(70,255,120,.16); border-color:rgba(70,255,120,.26); }
  .btn-blue{ background:rgba(70,140,255,.16); border-color:rgba(70,140,255,.26); }
  .btn-yellow{ background:rgba(255,230,80,.16); border-color:rgba(255,230,80,.26); color:#fff; }
  .btn-purple{ background:rgba(170,90,255,.16); border-color:rgba(170,90,255,.26); }
  .btn-status{ background:rgba(255,255,255,.08); }

  .swatch{
    display:flex;
    align-items:center;
    gap:10px;
  }
  .swatchbox{
    width:40px;height:40px;
    border-radius:12px;
    border:1px solid var(--border);
    background:#000;
    box-shadow: inset 0 0 0 1px rgba(255,255,255,.05);
  }
  .kv{
    display:flex;
    flex-direction:column;
    gap:2px;
  }
  .kv .k{ color:var(--muted); font-size:12px; }
  .kv .v{ font-size:14px; font-weight:650; }

  .bright{
    display:flex;
    gap:10px;
    flex-wrap:wrap;
    margin-top:10px;
  }
  .bright button{
    width:auto;
    min-width:110px;
  }

  #log{
  margin-top:10px;
  padding:12px;
  border-radius:12px;
  background:rgba(0,0,0,.28);
  border:1px solid var(--border);
  color:#e8eeff;
  font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", monospace;
  font-size:12px;
  line-height:1.35;
  white-space:pre-wrap;

  height: 180px;          /* or 220px */
  overflow-y: auto;
  overflow-x: hidden;
}
</style>
</head>

<body>
  <div class="wrap">
    <div class="header">
      <div>
        <h1>ESP32 WebUI Controls</h1>
        <p class="sub">This allows us to control the NeoPixel present at the ESP32's board, it also logs the actions taken</p>
      </div>
      <button class="btn-orange" style="max-width:100px" onclick="restartESP()">⚠️  Restart</button>
      <!--<div class="badge" title="UI is loaded">
        <span class="dot"></span>
        <strong>WebUI</strong>
      </div>-->
    </div>

    <div class="card">
      <div class="row">
        <div class="swatch">
          <div id="swatch" class="swatchbox"></div>
          <div class="kv">
            <div class="k">Current</div>
            <div id="cur" class="v">Unknown</div>
          </div>
        </div>
      </div>

      <div class="grid">
        <button class="btn-off" onclick="cmd('/off', 0,0,0, 'OFF')">OFF</button>
        <button class="btn-red" onclick="cmd('/red', 255,0,0, 'RED')">RED</button>
        <button class="btn-green" onclick="cmd('/green', 0,255,0, 'GREEN')">GREEN</button>
        <button class="btn-blue" onclick="cmd('/blue', 0,0,255, 'BLUE')">BLUE</button>
        <button class="btn-yellow" onclick="cmd('/yellow', 255,255,0, 'YELLOW')">YELLOW</button>
        <button class="btn-purple" onclick="cmd('/purple', 128,0,128, 'PURPLE')">PURPLE</button>
        <button class="btn-status" onclick="cmd('/status')">STATUS</button>
        <button class="btn-status" style="max-width:180px" onclick="clearLogs()">Clear Logs</button>
      </div>

      <div class="grid">
        <button class="btn-status" onclick="cmd('/b?set=10')">Brightness: Dim</button>
        <button class="btn-status" onclick="cmd('/b?set=50')">Brightness: 50</button>
        <button class="btn-status" onclick="cmd('/b?set=120')">Brightness: 120</button>
        <button class="btn-status" onclick="cmd('/b?set=255')">Brightness: Max</button>
      </div>

      <div id="log">Ready</div>
    </div>
  </div>

<script>
const logEl = document.getElementById('log');
const sw = document.getElementById('swatch');
const cur = document.getElementById('cur');


function setSwatch(r,g,b,label){
  if(typeof r === "number"){
    sw.style.background = `rgb(${r},${g},${b})`;
  }
  if(label) cur.textContent = label;
}

function log(msg){
  logEl.textContent = msg + "\n" + logEl.textContent;
}

function cmd(path, r, g, b, label){
  // optimistic UI update
  if(typeof r === "number") setSwatch(r,g,b,label);

  fetch(path, { cache: "no-store" })
    .then(r => r.text())
    .then(t => {
      log(t);
      // best-effort: parse "RGB(x,y,z)" from server response
      const m = t.match(/RGB\((\d+),(\d+),(\d+)\)/);
      if(m){
        setSwatch(parseInt(m[1]), parseInt(m[2]), parseInt(m[3]), label || cur.textContent);
      }
      if(!label && path === "/status"){
        // if status is pressed, show the RGB string as the label
        cur.textContent = m ? `RGB(${m[1]},${m[2]},${m[3]})` : "STATUS";
      }
    })
    .catch(e => log("ERR: " + e));
}

function clearLogs(){
  logEl.textContent = "";
  logEl.scrollTop = 0;
}

function restartESP(){
  if(!confirm("Restart ESP32 now?\n\nWifi signal will be lost")) return;

  log("Restarting ESP32...");
  fetch("/restart", { cache: "no-store" })
    .then(() => {
      log("ESP32 rebooting. Reconnect to Wi-Fi in a few seconds.");
    })
    .catch(() => {
      log("ESP32 rebooting...");
    });
}


// Load initial status
cmd('/status');
</script>
</body>
</html>
)HTML";

void setup() {
  Serial.begin(115200);

  // NeoPixel init
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show(); // off

  // Wi-Fi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  Serial.print("AP SSID: "); Serial.println(AP_SSID);
  Serial.print("AP IP: ");   Serial.println(WiFi.softAPIP());
  Serial.println("Open http://192.168.4.1/");

  // Web UI
  server.on("/", []() {
    server.send(200, "text/html; charset=utf-8", PAGE);
  });
  Serial.println("Server ON");

  // Color endpoint and action Logs ("rgbString")
  server.on("/off", []() {
    setColor(0, 0, 0);
    server.send(200, "text/plain", "OK OFF\n" + rgbString());
    Serial.println("OK OFF - " + rgbString());
  });

  server.on("/red", []() {
    setColor(255, 0, 0);
    server.send(200, "text/plain", "OK RED\n" + rgbString());
    Serial.println("OK RED - " + rgbString());
  });

  server.on("/green", []() {
    setColor(0, 255, 0);
    server.send(200, "text/plain", "OK GREEN\n" + rgbString());
    Serial.println("OK GREEN - " + rgbString());
  });

  server.on("/blue", []() {
    setColor(0, 0, 255);
    server.send(200, "text/plain", "OK BLUE\n" + rgbString());
    Serial.println("OK BLUE - " + rgbString());
  });

  server.on("/yellow", []() {
    setColor(255, 255, 0);
    server.send(200, "text/plain", "OK YELLOW\n" + rgbString());
    Serial.println("OK YELLOW - " + rgbString());
  });

  server.on("/purple", []() {
    setColor(128, 0, 128);
    server.send(200, "text/plain", "OK PURPLE\n" + rgbString());
    Serial.println("OK PURPLE - " + rgbString());
  });

  // Brightness endpoint: "/b?set=0..255"
  server.on("/b", []() {
    if (!server.hasArg("set")) {
      server.send(400, "text/plain", "ERR missing ?set=0..255");
      return;
    }
    int v = server.arg("set").toInt();
    if (v < 0) v = 0;
    if (v > 255) v = 255;

    BRIGHTNESS = (uint8_t)v;
    strip.setBrightness(BRIGHTNESS);
    strip.show(); // apply brightness

    server.send(200, "text/plain", "OK BRIGHTNESS\n" + rgbString());
    Serial.println("OK BRIGHTNESS - " + rgbString());
  });

  server.on("/status", []() {
    server.send(200, "text/plain", "OK STATUS\n" + rgbString());
    Serial.println("OK STATUS - " + rgbString());
  });

  server.begin();

  // Start with a known state
  setColor(0, 0, 0);

  // Restart ESP
  server.on("/restart", []() {
  server.send(200, "text/plain", "OK RESTARTING");
  Serial.println("Restart requested from WebUI");
  delay(500);          // allow response to flush
  ESP.restart();
});

}

void loop() {
  server.handleClient();
}