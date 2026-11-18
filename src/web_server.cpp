#include "web_server.h"
#include "web_assets.h"
#include <Update.h>

static const char OTA_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="ru">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Lunar Lamp — OTA Обновление</title>
  <style>
    :root {
      --bg: #090A0F;
      --card-bg: rgba(14, 18, 27, 0.85);
      --card-border: rgba(255, 255, 255, 0.08);
      --text: #F8FAFC;
      --text-muted: #94A3B8;
      --emerald: #10B981;
      --indigo: #818CF8;
      --red: #EF4444;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      background: var(--bg);
      background-image: radial-gradient(circle at 50% -10%, rgba(148, 163, 184, 0.09) 0%, transparent 60%);
      color: var(--text);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 20px;
    }
    .ota-card {
      width: 100%;
      max-width: 420px;
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: 20px;
      padding: 28px 24px;
      backdrop-filter: blur(20px);
      box-shadow: 0 20px 40px rgba(0,0,0,0.6);
      text-align: center;
    }
    .badge {
      display: inline-flex;
      align-items: center;
      gap: 6px;
      background: rgba(129, 140, 248, 0.12);
      border: 1px solid rgba(129, 140, 248, 0.3);
      color: var(--indigo);
      padding: 4px 10px;
      border-radius: 999px;
      font-size: 11px;
      font-weight: 600;
      letter-spacing: 0.08em;
      text-transform: uppercase;
      margin-bottom: 14px;
    }
    h1 { font-size: 20px; font-weight: 700; margin-bottom: 8px; }
    p { font-size: 13px; color: var(--text-muted); line-height: 1.5; margin-bottom: 24px; }
    .file-drop {
      display: block;
      border: 2px dashed rgba(255,255,255,0.15);
      border-radius: 12px;
      padding: 24px 16px;
      margin-bottom: 20px;
      cursor: pointer;
      transition: all 0.2s;
      background: rgba(0,0,0,0.25);
    }
    .file-drop:hover { border-color: rgba(255,255,255,0.3); background: rgba(255,255,255,0.02); }
    .file-name { font-size: 13px; font-weight: 600; color: #FFF; margin-top: 8px; word-break: break-all; }
    input[type="file"] { display: none; }
    .upload-btn {
      width: 100%;
      padding: 13px;
      border-radius: 12px;
      border: none;
      background: var(--indigo);
      color: #FFF;
      font-size: 14px;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.2s;
      box-shadow: 0 4px 14px rgba(99, 102, 241, 0.35);
    }
    .upload-btn:hover:not(:disabled) { background: #6366F1; transform: translateY(-1px); }
    .upload-btn:disabled { opacity: 0.4; cursor: not-allowed; }
    .progress-box { margin-top: 20px; display: none; text-align: left; }
    .progress-header { display: flex; justify-content: space-between; font-size: 12px; color: var(--text-muted); margin-bottom: 6px; }
    .progress-bar { width: 100%; height: 8px; border-radius: 999px; background: rgba(255,255,255,0.08); overflow: hidden; }
    .progress-fill { width: 0%; height: 100%; background: linear-gradient(90deg, var(--indigo), var(--emerald)); transition: width 0.15s; }
    .status-msg { margin-top: 14px; font-size: 12.5px; line-height: 1.4; }
    .back-link { display: inline-block; margin-top: 24px; font-size: 12px; color: var(--text-muted); text-decoration: none; transition: color 0.2s; }
    .back-link:hover { color: #FFF; }
  </style>
</head>
<body>
  <div class="ota-card">
    <div class="badge">ESP32 Core</div>
    <h1>Обновление прошивки</h1>
    <p>Выберите скомпилированный бинарный файл <code>Moon.ino.bin</code> для загрузки по воздуху.</p>
    
    <label class="file-drop" for="fileInput">
      <svg width="32" height="32" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" style="color:var(--indigo);">
        <path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/>
        <polyline points="17 8 12 3 7 8"/>
        <line x1="12" y1="3" x2="12" y2="15"/>
      </svg>
      <div class="file-name" id="fileName">Нажмите для выбора .bin файла</div>
    </label>
    <input type="file" id="fileInput" accept=".bin">

    <button id="uploadBtn" class="upload-btn" type="button" disabled>Установить обновление</button>

    <div id="progressBox" class="progress-box">
      <div class="progress-header">
        <span id="progressStatus">Загрузка...</span>
        <span id="progressPercent">0%</span>
      </div>
      <div class="progress-bar">
        <div id="progressFill" class="progress-fill"></div>
      </div>
      <div id="statusMsg" class="status-msg"></div>
    </div>

    <a href="/" class="back-link">← Вернуться к управлению лампой</a>
  </div>

  <script>
    const fileInput = document.getElementById('fileInput');
    const fileName = document.getElementById('fileName');
    const uploadBtn = document.getElementById('uploadBtn');
    const progressBox = document.getElementById('progressBox');
    const progressFill = document.getElementById('progressFill');
    const progressPercent = document.getElementById('progressPercent');
    const progressStatus = document.getElementById('progressStatus');
    const statusMsg = document.getElementById('statusMsg');

    fileInput.addEventListener('change', () => {
      if (fileInput.files.length > 0) {
        fileName.textContent = fileInput.files[0].name;
        uploadBtn.disabled = false;
      } else {
        fileName.textContent = "Нажмите для выбора .bin файла";
        uploadBtn.disabled = true;
      }
    });

    uploadBtn.addEventListener('click', () => {
      const file = fileInput.files[0];
      if (!file) return;

      uploadBtn.disabled = true;
      progressBox.style.display = 'block';
      statusMsg.innerHTML = '<span style="color: #94A3B8;">Запись прошивки в Flash-память... Пожалуйста, не выключайте питание лампы.</span>';

      const xhr = new XMLHttpRequest();
      xhr.open('POST', '/update', true);

      xhr.upload.onprogress = (e) => {
        if (e.lengthComputable) {
          const percent = Math.round((e.loaded / e.total) * 100);
          progressFill.style.width = percent + '%';
          progressPercent.textContent = percent + '%';
          if (percent === 100) {
            progressStatus.textContent = 'Проверка и запись...';
          }
        }
      };

      xhr.onload = () => {
        if (xhr.status === 200) {
          progressFill.style.width = '100%';
          progressPercent.textContent = '100%';
          progressStatus.textContent = 'Готово!';
          statusMsg.innerHTML = '<span style="color: #10B981; font-weight: 600;">✓ Прошивка успешно обновлена! Лампа перезагружается...</span>';
          setTimeout(() => { window.location.href = '/'; }, 12000);
        } else {
          statusMsg.innerHTML = '<span style="color: #EF4444; font-weight: 600;">✕ Ошибка обновления: ' + xhr.responseText + '</span>';
          uploadBtn.disabled = false;
        }
      };

      xhr.onerror = () => {
        statusMsg.innerHTML = '<span style="color: #EF4444; font-weight: 600;">✕ Ошибка соединения при загрузке.</span>';
        uploadBtn.disabled = false;
      };

      const formData = new FormData();
      formData.append('update', file);
      xhr.send(formData);
    });
  </script>
</body>
</html>
)rawliteral";

LampWebServer::LampWebServer(LedEngine& ledEngine, AstronomyEngine& astroEngine, SettingsManager& settings)
    : _httpServer(80), _wsServer(81), _leds(ledEngine), _astro(astroEngine), _settings(settings), _lastTeleBroadcast(0) {}

void LampWebServer::begin() {
    // HTTP маршрут отдачи сжатого веб-интерфейса
    _httpServer.on("/", HTTP_GET, [this]() {
        this->handleRoot();
    });

    // Маршрут веб-страницы обновления OTA
    _httpServer.on("/update", HTTP_GET, [this]() {
        this->handleOtaPage();
    });

    // Обработчик загрузки файла обновления OTA
    _httpServer.on("/update", HTTP_POST, [this]() {
        _httpServer.sendHeader("Connection", "close");
        if (!Update.hasError()) {
            _httpServer.send(200, "text/plain", "OK");
            delay(500);
            ESP.restart();
        } else {
            _httpServer.send(500, "text/plain", Update.errorString());
        }
    }, [this]() {
        HTTPUpload& upload = _httpServer.upload();
        if (upload.status == UPLOAD_FILE_START) {
            Serial.printf("[OTA] Update upload started: %s\n", upload.filename.c_str());
            FastLED.clear(true);
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                Serial.printf("[OTA ERR] Update.begin failed: %s\n", Update.errorString());
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Serial.printf("[OTA ERR] Write failed: %s\n", Update.errorString());
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) {
                Serial.printf("[OTA OK] Update successful! %u bytes written. Rebooting...\n", upload.totalSize);
            } else {
                Serial.printf("[OTA ERR] Update.end failed: %s\n", Update.errorString());
            }
        }
    });

    _httpServer.onNotFound([this]() {
        _httpServer.sendHeader("Location", "/");
        _httpServer.send(302, "text/plain", "");
    });

    _httpServer.begin();

    // Запуск WebSockets сервера на порту 81
    _wsServer.begin();
    _wsServer.onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
        this->onWebSocketEvent(num, type, payload, length);
    });

    Serial.println("[OK] HTTP Web Server started on port 80");
    Serial.println("[OK] WebSockets Server started on port 81");
    Serial.println("[OK] OTA Web Updater available at /update");
}

void LampWebServer::handleRoot() {
    _httpServer.sendHeader("Content-Encoding", "gzip");
    _httpServer.send_P(200, "text/html", (const char*)INDEX_HTML_GZ, INDEX_HTML_GZ_LEN);
}

void LampWebServer::handleOtaPage() {
    _httpServer.send_P(200, "text/html", OTA_HTML);
}

void LampWebServer::onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED: {
            IPAddress ip = _wsServer.remoteIP(num);
            Serial.printf("[WS] Client #%u connected from %s\n", num, ip.toString().c_str());
            broadcastState();
            break;
        }
        case WStype_DISCONNECTED:
            Serial.printf("[WS] Client #%u disconnected\n", num);
            break;

        case WStype_TEXT: {
            String msg = String((char*)payload).substring(0, length);
            handleJsonCommand(num, msg);
            break;
        }
        default:
            break;
    }
}

void LampWebServer::handleJsonCommand(uint8_t clientNum, const String& jsonStr) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, jsonStr);
    if (err) {
        Serial.printf("[WS ERR] JSON parse failed: %s\n", err.c_str());
        return;
    }

    const char* cmd = doc["cmd"] | "";

    if (strcmp(cmd, "getState") == 0) {
        broadcastState();
    } else if (strcmp(cmd, "setPower") == 0) {
        _leds.setPower(doc["power"] | true);
        broadcastState();
        _settings.scheduleSave();
    } else if (strcmp(cmd, "setBrightness") == 0) {
        _leds.setBrightness(doc["brightness"] | 180);
        broadcastState();
        _settings.scheduleSave();
    } else if (strcmp(cmd, "setKelvin") == 0) {
        _leds.setKelvin(doc["kelvin"] | 4000);
        broadcastState();
        _settings.scheduleSave();
    } else if (strcmp(cmd, "setMode") == 0) {
        _leds.setMode((LampMode)(doc["mode"] | 0));
        broadcastState();
        _settings.scheduleSave();
    } else if (strcmp(cmd, "setManualPhase") == 0) {
        _leds.setManualPhase(doc["angle"] | 180.0);
        broadcastState();
        _settings.scheduleSave();
    } else if (strcmp(cmd, "setLedCount") == 0) {
        _leds.setLedCount(doc["count"] | 252);
        broadcastState();
        _settings.scheduleSave();
    } else if (strcmp(cmd, "setOffsetAngle") == 0) {
        _leds.setZeroAngleOffset(doc["offset"] | 0);
        broadcastState();
        _settings.scheduleSave();
    } else if (strcmp(cmd, "setAutoSunset") == 0) {
        _leds.setAutoSunset(doc["enabled"] | true);
        broadcastState();
        _settings.scheduleSave();
    } else if (strcmp(cmd, "setAutoOff") == 0) {
        _leds.setAutoOff(doc["enabled"] | true);
        broadcastState();
        _settings.scheduleSave();
    } else if (strcmp(cmd, "setAutoOffTime") == 0) {
        uint8_t h = doc["hour"] | 1;
        uint8_t m = doc["minute"] | 0;
        _leds.setAutoOffTime(h, m);
        broadcastState();
        _settings.scheduleSave();
    } else if (strcmp(cmd, "resetDefaults") == 0) {
        _settings.resetToDefaults(_leds, _astro);
        broadcastState();
    }
}

void LampWebServer::broadcastState() {
    LampState s = _leds.getState();

    JsonDocument doc;
    JsonObject stateObj = doc["state"].to<JsonObject>();
    stateObj["power"] = s.power;
    stateObj["brightness"] = s.brightness;
    stateObj["kelvin"] = s.kelvin;
    stateObj["mode"] = (int)s.mode;
    stateObj["manualAngle"] = s.manualPhaseAngle;
    stateObj["ledCount"] = s.ledCount;
    stateObj["offsetAngle"] = s.zeroAngleOffset;
    stateObj["autoSunset"] = s.autoSunset;
    stateObj["autoOff"] = s.autoOff;
    stateObj["autoOffHour"] = s.autoOffHour;
    stateObj["autoOffMinute"] = s.autoOffMinute;

    String out;
    serializeJson(doc, out);
    _wsServer.broadcastTXT(out);
}

void LampWebServer::broadcastTelemetry(const MoonTelemetry& moon, const SunTelemetry& sun) {
    JsonDocument doc;
    JsonObject tele = doc["telemetry"].to<JsonObject>();

    tele["illumination"] = moon.illumination;
    tele["phaseAngle"] = moon.phaseAngleDeg;
    tele["isWaxing"] = moon.isWaxing;
    tele["phaseName"] = moon.phaseNameRu;
    tele["ageDays"] = moon.ageDays;

    char sunBuf[16];
    snprintf(sunBuf, sizeof(sunBuf), "%02d:%02d", sun.sunriseHour, sun.sunriseMinute);
    tele["sunrise"] = String(sunBuf);

    snprintf(sunBuf, sizeof(sunBuf), "%02d:%02d", sun.sunsetHour, sun.sunsetMinute);
    tele["sunset"] = String(sunBuf);

    tele["dayPhase"] = sun.dayPhaseNameRu;

    String out;
    serializeJson(doc, out);
    _wsServer.broadcastTXT(out);
}

void LampWebServer::update(const MoonTelemetry& moon, const SunTelemetry& sun) {
    _httpServer.handleClient();
    _wsServer.loop();

    unsigned long now = millis();
    // Рассылка астрономической телеметрии клиентам каждые 10 секунд
    if (now - _lastTeleBroadcast >= 10000) {
        _lastTeleBroadcast = now;
        broadcastTelemetry(moon, sun);
    }
}
