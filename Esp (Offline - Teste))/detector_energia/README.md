# Detector de Queda de Energia — ESP32-C6
**Projeto Integrador II | UFSM**

Angelo Rosa Tonetto · Daniela Alessandra Prill · Julia Blanco Garcia · Thomas Patrick Burrow

---

## Visão Geral

Firmware para ESP32-C6 que monitora tensão, corrente e frequência da rede elétrica usando
um sensor **PZEM-004T** via UART. Os dados são publicados via **MQTT** no formato
**Influx Line Protocol**, prontos para ingestão no **InfluxDB** e visualização em dashboard.

Quando não há conectividade, os dados ficam em um **buffer circular** na RAM e são
enviados automaticamente assim que a conexão retornar.

Suporta atualização de firmware via **ElegantOTA** (sem precisar de cabo USB).

---

## Requisitos de Hardware

| Componente | Detalhe |
|---|---|
| ESP32-C6 | DevKitC-1 ou equivalente |
| PZEM-004T v3.0 | Sensor de tensão/corrente/freq. |
| Fonte 5 V | Para alimentar o ESP32-C6 |
| Transformador de corrente (TC) | Incluído no kit PZEM |

### Fiação UART (ajuste em `config.h` se necessário)

```
ESP32-C6  ←→  PZEM-004T
GPIO4 (RX) ←  TX do PZEM
GPIO5 (TX) →  RX do PZEM
GND        —  GND do PZEM
```

> **Atenção:** O PZEM-004T deve ser conectado à rede elétrica de 127/220 V pelo
> conector próprio. Siga as instruções de segurança do datasheet.

---

## Configuração do Projeto

### 1. Editar `src/config.h`

```c
// WiFi
#define WIFI_SSID       "NOME_DA_REDE"
#define WIFI_PASSWORD   "SENHA_DA_REDE"

// MQTT (broker InfluxDB ou Mosquitto)
#define MQTT_SERVER     "192.168.x.x"
#define MQTT_PORT       1883
#define MQTT_USER       "usuario"
#define MQTT_PASSWORD   "senha"
#define MQTT_TOPIC      "ufsm/laboratorio/energia"

// Tag de localização no InfluxDB
#define INFLUX_LOCATION_TAG  "lab_ufsm"
```

### 2. Compilar e gravar via PlatformIO

```bash
# Instalar dependências e gravar pela primeira vez (USB)
pio run --target upload

# Monitor serial
pio device monitor --baud 115200
```

### 3. Atualizações OTA (sem USB)

Após a primeira gravação, acesse:
```
http://<IP_DO_ESP>/update
```
Faça upload do arquivo `.pio/build/esp32-c6-devkitc-1/firmware.bin`.

---

## Integração com InfluxDB via MQTT

Configure o **Telegraf** para consumir o tópico MQTT e escrever no InfluxDB:

```toml
# telegraf.conf (trecho)
[[inputs.mqtt_consumer]]
  servers = ["tcp://localhost:1883"]
  topics  = ["ufsm/laboratorio/energia"]
  data_format = "influx"

[[outputs.influxdb_v2]]
  urls   = ["http://localhost:8086"]
  token  = "SEU_TOKEN"
  org    = "ufsm"
  bucket = "laboratorio"
```

### Exemplo de linha gerada pelo firmware

```
energia,location=lab_ufsm voltage=219.80,current=1.253,frequency=60.0,power=275.44,energy=0.0031,power_factor=0.998 1714406400000000000
```

---

## Fluxo do Firmware

```
SETUP
  └─ Conectar WiFi (loop)
  └─ Iniciar NTP
  └─ Configurar MQTT
  └─ Iniciar ElegantOTA

LOOP (a cada 5 s)
  ├─ WiFi conectado? → se não: reconectar
  ├─ Ler PZEM-004T via UART
  ├─ NTP sincronizado?
  │   ├─ Sim → salvar {V, I, F, P, E, FP, timestamp} no buffer
  │   └─ Não → descartar leitura
  ├─ MQTT conectado? → se não: conectar
  ├─ Buffer com dados?
  │   ├─ Sim + MQTT OK → enviar (Influx Line Protocol) → remover do buffer
  │   └─ Não / falha   → manter no buffer
  ├─ Atender clientes HTTP (ElegantOTA)
  └─ "Funcionando."
```

---

## Parâmetros Ajustáveis (`config.h`)

| Parâmetro | Padrão | Descrição |
|---|---|---|
| `LOOP_INTERVAL_MS` | 5000 | Intervalo entre leituras (ms) |
| `BUFFER_MAX_SIZE` | 500 | Máx. de leituras em RAM (~14 KB) |
| `BUFFER_SEND_PER_LOOP` | 10 | Entradas enviadas por ciclo |
| `WIFI_RECONNECT_TIMEOUT_MS` | 10000 | Timeout para reconexão WiFi |
| `NTP_GMT_OFFSET_SEC` | -10800 | UTC-3 (Brasília) |
| `SENSOR_UART_RX_PIN` | 4 | GPIO RX do PZEM |
| `SENSOR_UART_TX_PIN` | 5 | GPIO TX do PZEM |

---

## Detecção de Queda de Energia

A duração da queda é calculada **no servidor** (InfluxDB/dashboard):

1. Quando a energia cai, o ESP perde conectividade e para de enviar dados.
2. O **último timestamp antes da queda** e o **primeiro timestamp após o retorno**
   ficam registrados no InfluxDB.
3. A diferença entre eles é a duração da falta de energia.
4. O buffer garante que dados do período de instabilidade (pré/pós queda) não sejam perdidos.

---

## Dependências (PlatformIO)

| Biblioteca | Versão |
|---|---|
| `knolleary/PubSubClient` | ^2.8 |
| `ayushsharma82/ElegantOTA` | ^3.1.6 |
| `olehs/PZEM004Tv30` | ^1.1.2 |
| `arduino-libraries/NTPClient` | ^3.2.1 |
