#pragma once

// ============================================================
//  secrets.h - Credenciais REAIS (NAO sobe pro GitHub)
//  Este arquivo está no .gitignore.
//
//  Cada integrante do grupo deve preencher com a própria rede
//  WiFi pra testar localmente.
// ============================================================

// --- WiFi ---
// Troque pela sua rede (casa, hotspot do celular, etc.)
#define WIFI_SSID       "NOME_DA_SUA_REDE"
#define WIFI_PASSWORD   "SENHA_DA_SUA_REDE"

// --- MQTT real (usado só quando TEST_MODE = false) ---
#define MQTT_SERVER_REAL    "ENDERECO_DO_SERVIDOR_UFSM"
#define MQTT_PORT_REAL      1883
#define MQTT_USER_REAL      "USUARIO_MQTT"
#define MQTT_PASSWORD_REAL  "SENHA_MQTT"
