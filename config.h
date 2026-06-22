#pragma once

// ============================================================
//  secrets.h.example - MODELO de credenciais
//  Detector de Queda de Energia | UFSM
//
//  COMO USAR:
//   1. Copie este arquivo e renomeie a cópia para "secrets.h"
//      (na mesma pasta, src/)
//   2. Preencha os valores reais abaixo no secrets.h
//   3. O secrets.h NUNCA vai pro GitHub (está no .gitignore)
//      — cada integrante do grupo cria o seu próprio.
// ============================================================

// --- WiFi ---
#define WIFI_SSID       "NOME_DA_SUA_REDE"
#define WIFI_PASSWORD   "SENHA_DA_SUA_REDE"

// --- MQTT ---
// Em TEST_MODE (config.h) o broker público já é usado
// automaticamente — só precisa preencher isso quando for
// usar o servidor real da UFSM (TEST_MODE = false).
#define MQTT_SERVER_REAL    "ENDERECO_DO_SERVIDOR_UFSM"
#define MQTT_PORT_REAL      1883
#define MQTT_USER_REAL      "USUARIO_MQTT"
#define MQTT_PASSWORD_REAL  "SENHA_MQTT"
