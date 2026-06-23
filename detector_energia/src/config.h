#pragma once

// ===========================================================
//  CONFIGURAÇÕES DO PROJETO - Detector de Queda de Energia
//  UFSM - Projeto Integrador II
//
//  Este arquivo NÃO contém senhas/credenciais — pode subir
//  pro GitHub sem problema. As credenciais reais ficam em
//  src/secrets.h (gitignored, cada um cria o seu).
// ===========================================================

#include "secrets.h"

// -----------------------------------------------------------
//  MODO DE TESTE
//  true  → gera dados simulados, não precisa do PZEM físico,
//          usa o broker público HiveMQ
//  false → lê o sensor PZEM-004T real via UART,
//          usa o broker/servidor da UFSM (de secrets.h)
// -----------------------------------------------------------
#define TEST_MODE false

// -----------------------------------------------------------
//  WiFi (valores vêm de secrets.h)
// -----------------------------------------------------------
#define WIFI_RECONNECT_TIMEOUT_MS  10000

// -----------------------------------------------------------
//  MQTT
//
//  MODO TESTE → broker público gratuito (sem senha, sem config)
//    Host : broker.hivemq.com
//    Porta: 1883
//
//  MODO REAL → usa os dados de secrets.h (MQTT_SERVER_REAL etc.)
// -----------------------------------------------------------
#if TEST_MODE
  #define MQTT_SERVER    "broker.hivemq.com"
  #define MQTT_PORT      1883
  #define MQTT_USER      ""
  #define MQTT_PASSWORD  ""
  // Tópico único pra não conflitar com outros usuários do broker
  // Troque "grupo04" se quiser
  #define MQTT_TOPIC     "ufsm/pi2/grupo04/energia"
#else
  #define MQTT_SERVER    MQTT_SERVER_REAL
  #define MQTT_PORT      MQTT_PORT_REAL
  #define MQTT_USER      MQTT_USER_REAL
  #define MQTT_PASSWORD  MQTT_PASSWORD_REAL
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
