#pragma once

// -----------------------------------------------------------
//  MODO DE TESTE
//  true  → gera dados simulados, não precisa do PZEM físico
//  false → lê o sensor PZEM-004T real via UART
// -----------------------------------------------------------
#define TEST_MODE true

// -----------------------------------------------------------
//  WiFi
//  Durante testes: use sua rede doméstica/hotspot normalmente
// -----------------------------------------------------------
#define WIFI_SSID       "NOME_DA_REDE"
#define WIFI_PASSWORD   "SENHA_DA_REDE"
#define WIFI_RECONNECT_TIMEOUT_MS  10000

// -----------------------------------------------------------
//  MQTT
//
//  MODO TESTE → broker público gratuito (sem senha, sem config)
//    Host : broker.hivemq.com
//    Porta: 1883
//    User : (vazio)
//    Pass : (vazio)
//
//  MODO REAL → substitua pelos dados da UFSM
// -----------------------------------------------------------
#if TEST_MODE
  #define MQTT_SERVER    "broker.hivemq.com"
  #define MQTT_PORT      1883
  #define MQTT_USER      ""
  #define MQTT_PASSWORD  ""
  // Tópico único pra não conflitar com outros usuários do broker
  // Troque "grupo04" por algo seu
  #define MQTT_TOPIC     "ufsm/pi2/grupo04/energia"
#else
  #define MQTT_SERVER    "SEU_SERVIDOR_MQTT"
  #define MQTT_PORT      1883
  #define MQTT_USER      "USUARIO_MQTT"
  #define MQTT_PASSWORD  "SENHA_MQTT"
  #define MQTT_TOPIC     "ufsm/laboratorio/energia"
#endif

#define MQTT_CLIENT_ID  "esp32c6-detector-energia"

// -----------------------------------------------------------
//  NTP
// -----------------------------------------------------------
#define NTP_SERVER          "pool.ntp.org"
#define NTP_GMT_OFFSET_SEC  -10800   // UTC-3 (Brasília)
#define NTP_DST_OFFSET_SEC  0

// -----------------------------------------------------------
//  UART do Sensor PZEM-004T (ignorado em TEST_MODE)
// -----------------------------------------------------------
#define SENSOR_UART_RX_PIN  4
#define SENSOR_UART_TX_PIN  5
#define SENSOR_UART_BAUD    9600
#define SENSOR_UART_NUM     1
#define PZEM_ADDRESS        PZEM_DEFAULT_ADDR

// -----------------------------------------------------------
//  Buffer circular
// -----------------------------------------------------------
#define BUFFER_MAX_SIZE      500
#define BUFFER_SEND_PER_LOOP  10

// -----------------------------------------------------------
//  Loop principal
// -----------------------------------------------------------
#define LOOP_INTERVAL_MS  3000   // 3s no teste pra ver dados rápido

// -----------------------------------------------------------
//  ElegantOTA
// -----------------------------------------------------------
#define OTA_HTTP_PORT  80

// -----------------------------------------------------------
//  Influx Line Protocol
//  Formato: sensor,sensor_id=<ID> volt=V,amp=A,freq=F <ts>
// -----------------------------------------------------------
#define INFLUX_MEASUREMENT  "sensor"
#define INFLUX_SENSOR_ID    "lab_ufsm_teste"
