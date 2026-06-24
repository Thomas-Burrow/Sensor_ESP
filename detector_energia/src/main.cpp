/**
 * ============================================================
 *  Detector de Queda de Energia - ESP32-C6
 *  Projeto Integrador II | UFSM
 *
 *  Autores: Angelo Rosa Tonetto, Daniela Alessandra Prill,
 *           Julia Blanco Garcia, Thomas Patrick Burrow
 *
 *  Linguagem: C++ (Arduino framework via PlatformIO)
 *
 *  Formato Influx Line Protocol:
 *    sensor,sensor_id=lab_ufsm volt=219.80,amp=1.253,freq=60.0 1714406400
 *
 *  TEST_MODE=true  → dados simulados, broker público HiveMQ
 *  TEST_MODE=false → PZEM-004T real, broker da UFSM
 *
 *  Credenciais ficam em src/secrets.h (não sobe pro GitHub).
 * ============================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ElegantOTA.h>
#include <PubSubClient.h>
#include <time.h>
#include <deque>

#include "config.h"   // já inclui secrets.h por dentro

// Só inclui a lib do PZEM quando não está em modo de teste
#if !TEST_MODE
  #include <PZEM004Tv30.h>
#endif

// ===========================================================
//  ESTRUTURA DE DADOS
// ===========================================================

struct SensorData {
    float  voltage;    // volt
    float  current;    // amp
    float  frequency;  // freq
    time_t timestamp;  // segundos Unix
};

// ===========================================================
//  OBJETOS GLOBAIS
// ===========================================================

#if !TEST_MODE
  HardwareSerial pzemSerial(SENSOR_UART_NUM);
  PZEM004Tv30    pzem(pzemSerial, 3, 2, PZEM_ADDRESS);
#endif

WebServer    httpServer(OTA_HTTP_PORT);
WiFiClient   wifiClient;
PubSubClient mqttClient(wifiClient);

std::deque<SensorData> dataBuffer;
unsigned long lastLoopTime = 0;

// ===========================================================
//  GERADOR DE DADOS SIMULADOS (TEST_MODE)
// ===========================================================

#if TEST_MODE

/**
 * Estados da simulação de rede elétrica.
 * Permite testar o comportamento do buffer durante uma queda.
 */
enum class RedeState {
    NORMAL,         // rede ok, tensão estável ~220V
    INSTAVEL,       // tensão oscilando (pré-queda)
    QUEDA,          // sem energia: readSensor retorna false
    RETORNO         // energia voltando, tensão subindo
};

static RedeState  redeState      = RedeState::NORMAL;
static int        ciclosNoEstado = 0;  // quantos loops no estado atual

/**
 * Simula uma leitura do sensor com dados realistas.
 * Também simula ciclos de queda de energia pra testar o buffer.
 *
 * Ciclo completo (aproximado):
 *   20 leituras NORMAL → 5 INSTAVEL → 8 QUEDA → 5 RETORNO → NORMAL ...
 */
bool readSensor(SensorData &out) {
    ciclosNoEstado++;

    switch (redeState) {
        case RedeState::NORMAL:
            if (ciclosNoEstado > 20) {
                redeState = RedeState::INSTAVEL;
                ciclosNoEstado = 0;
                Serial.println("[SIM] Rede começando a oscilar...");
            }
            break;

        case RedeState::INSTAVEL:
            if (ciclosNoEstado > 5) {
                redeState = RedeState::QUEDA;
                ciclosNoEstado = 0;
                Serial.println("[SIM] *** QUEDA DE ENERGIA SIMULADA ***");
            }
            break;

        case RedeState::QUEDA:
            if (ciclosNoEstado > 8) {
                redeState = RedeState::RETORNO;
                ciclosNoEstado = 0;
                Serial.println("[SIM] Energia retornando...");
            }
            return false;  // durante a queda, sensor não lê nada

        case RedeState::RETORNO:
            if (ciclosNoEstado > 5) {
                redeState = RedeState::NORMAL;
                ciclosNoEstado = 0;
                Serial.println("[SIM] Rede estabilizada.");
            }
            break;
    }

    float baseVolt, baseAmp, baseFreq;

    switch (redeState) {
        case RedeState::NORMAL:
            baseVolt = 219.0f + (random(-20, 20) * 0.1f);
            baseAmp  =   1.2f + (random(-10, 10) * 0.01f);
            baseFreq =  60.0f + (random(-2, 2)   * 0.1f);
            break;

        case RedeState::INSTAVEL:
            baseVolt = 200.0f + (random(-50, 50) * 0.5f);
            baseAmp  =   1.5f + (random(-30, 30) * 0.05f);
            baseFreq =  59.5f + (random(-10, 10) * 0.1f);
            break;

        case RedeState::RETORNO:
            baseVolt = 150.0f + (ciclosNoEstado * 14.0f);
            baseAmp  =   0.5f + (ciclosNoEstado * 0.15f);
            baseFreq =  59.0f + (ciclosNoEstado * 0.2f);
            break;

        default:
            baseVolt = 220.0f;
            baseAmp  = 1.2f;
            baseFreq = 60.0f;
    }

    out.voltage   = baseVolt;
    out.current   = baseAmp;
    out.frequency = baseFreq;
    out.timestamp = 0;

    return true;
}

#else
// ===========================================================
//  LEITURA REAL — PZEM-004T via UART
// ===========================================================

bool readSensor(SensorData &out) {
    float v = pzem.voltage();
    float c = pzem.current();
    float f = pzem.frequency();
    char test[50];
    snprintf(test, sizeof(test), "v = %.2f c = %.2f f = %.2f");

    Serial.println(test);

    if (isnan(v) || isnan(c) || isnan(f)) {
        //O sensor usa a energia AC para funcionar, portanto quando a leitura falha devemos mostrar 0
        v = 0.0;
        c = 0.0;
        f = 0.0;
    }

    out.voltage   = v;
    out.current   = c;
    out.frequency = f;
    out.timestamp = 0;

    return true;
}

#endif  // TEST_MODE

// ===========================================================
//  FUNÇÕES DE REDE
// ===========================================================

void setupWiFi() {
    Serial.printf("[WiFi] Conectando a '%s'", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.printf("\n[WiFi] Conectado! IP: %s\n",
                  WiFi.localIP().toString().c_str());
}

void reconnectWiFi() {
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < WIFI_RECONNECT_TIMEOUT_MS) {
        delay(300);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n[WiFi] Reconectado! IP: %s\n",
                      WiFi.localIP().toString().c_str());
    } else {
        Serial.println("\n[WiFi] Falha. Continuando offline.");
    }
}

void connectMQTT() {
    if (WiFi.status() != WL_CONNECTED) return;

    Serial.printf("[MQTT] Conectando a %s...", MQTT_SERVER);

    String clientId = String(MQTT_CLIENT_ID) + "-" +
                      String((uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF), HEX);

    bool ok;
    if (strlen(MQTT_USER) > 0) {
        ok = mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD);
    } else {
        ok = mqttClient.connect(clientId.c_str());  // broker sem autenticação
    }

    if (ok) {
        Serial.println(" OK!");
    } else {
        Serial.printf(" Falha (estado=%d).\n", mqttClient.state());
    }
}

// ===========================================================
//  INFLUX LINE PROTOCOL
// ===========================================================

String buildInfluxLine(const SensorData &d) {
    char line[200];
    snprintf(line, sizeof(line),
        "%s,sensor_id=%s volt=%.2f,amp=%.3f,freq=%.1f %ld",
        INFLUX_MEASUREMENT,
        INFLUX_SENSOR_ID,
        d.voltage,
        d.current,
        d.frequency,
        (long)d.timestamp
    );
    return String(line);
}

// ===========================================================
//  BUFFER → MQTT
// ===========================================================

void drainBuffer() {
    if (dataBuffer.empty()) {
        Serial.println("[BUFFER] Vazio.");
        return;
    }

    if (!mqttClient.connected()) {
        Serial.printf("[MQTT] Desconectado. %d entrada(s) no buffer.\n",
                      (int)dataBuffer.size());
        return;
    }

    int sent    = 0;
    int maxSend = min((int)dataBuffer.size(), BUFFER_SEND_PER_LOOP);

    while (sent < maxSend && !dataBuffer.empty()) {
        String msg = buildInfluxLine(dataBuffer.front());

        if (mqttClient.publish(MQTT_TOPIC, msg.c_str())) {
            Serial.println("[MQTT] >> " + msg);
            dataBuffer.pop_front();
            sent++;
        } else {
            Serial.println("[MQTT] Falha. Tentara depois.");
            break;
        }

        delay(20);
        mqttClient.loop();
    }

    Serial.printf("[BUFFER] %d enviado(s). %d restante(s).\n",
                  sent, (int)dataBuffer.size());
}

// ===========================================================
//  SETUP
// ===========================================================

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n================================================");
    Serial.println("  Detector de Queda de Energia - ESP32-C6");
    Serial.println("  UFSM - Projeto Integrador II");
#if TEST_MODE
    Serial.println("  *** MODO DE TESTE ATIVO ***");
    Serial.println("  Broker: broker.hivemq.com (publico)");
    Serial.printf ("  Topico: %s\n", MQTT_TOPIC);
#endif
    Serial.println("================================================\n");

    setupWiFi();

    configTime(NTP_GMT_OFFSET_SEC, NTP_DST_OFFSET_SEC, NTP_SERVER);
    Serial.println("[NTP] Sincronizando...");

    time_t now = time(nullptr);
    int tentativas = 0;
    while (now < 1000000000UL && tentativas < 20) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
        tentativas++;
    }
    Serial.printf("\n[NTP] Timestamp: %ld\n", (long)now);

    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setBufferSize(512);

    httpServer.on("/", []() {
        httpServer.send(200, "text/plain",
            "Detector de Queda de Energia - OK\n"
            "OTA: /update\n"
            "Topico MQTT: " MQTT_TOPIC);
    });
    ElegantOTA.begin(&httpServer);
    httpServer.begin();
    Serial.printf("[OTA] http://%s/update\n",
                  WiFi.localIP().toString().c_str());

    Serial.println("\n[SISTEMA] Iniciando loop...\n");
}

// ===========================================================
//  LOOP PRINCIPAL
// ===========================================================

void loop() {
    unsigned long now = millis();
    if (now - lastLoopTime < LOOP_INTERVAL_MS) {
        httpServer.handleClient();
        ElegantOTA.loop();
        mqttClient.loop();
        return;
    }
    lastLoopTime = now;

    Serial.println("──────────────────────────────────────");

    // 1. WiFi
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WiFi] Reconectando...");
        reconnectWiFi();
    }

    // 2. Ler sensor (real ou simulado)
    SensorData data;
    bool sensorOk = readSensor(data);

    // 3. Timestamp + buffer
    if (sensorOk) {
        time_t ts = time(nullptr);
        if (ts > 1000000000UL) {
            data.timestamp = ts;

            if (dataBuffer.size() >= BUFFER_MAX_SIZE) {
                dataBuffer.pop_front();
                Serial.println("[BUFFER] Cheio, removendo mais antigo.");
            }

            dataBuffer.push_back(data);
            Serial.printf("[SENSOR] volt=%.2f amp=%.3f freq=%.1f ts=%ld\n",
                          data.voltage, data.current, data.frequency,
                          (long)data.timestamp);
            Serial.printf("[BUFFER] %d entrada(s).\n", (int)dataBuffer.size());
        } else {
            Serial.println("[NTP] Sem timestamp. Descartando leitura.");
        }
    } else {
        Serial.println("[SENSOR] Sem leitura (queda de energia?).");
        Serial.printf("[BUFFER] %d entrada(s) preservadas.\n",
                      (int)dataBuffer.size());
    }

    // 4. MQTT
    if (!mqttClient.connected()) connectMQTT();
    drainBuffer();

    // 5. OTA
    httpServer.handleClient();
    ElegantOTA.loop();
    mqttClient.loop();

    Serial.println("[SISTEMA] Funcionando.");
}
