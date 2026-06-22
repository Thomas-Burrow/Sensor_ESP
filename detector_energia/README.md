# Detector de Queda de Energia — ESP32-C6
**Projeto Integrador II | UFSM**

Angelo Rosa Tonetto · Daniela Alessandra Prill · Julia Blanco Garcia · Thomas Patrick Burrow

---

## Visão Geral

Firmware para ESP32-C6 que monitora tensão, corrente e frequência da rede elétrica usando
um sensor **PZEM-004T** via UART. Os dados são publicados via **MQTT** no formato
**Influx Line Protocol**, prontos para ingestão no **InfluxDB**.

Tem um **modo de teste** embutido (`TEST_MODE`) que simula dados e ciclos de queda
de energia, sem precisar do sensor físico nem da rede da UFSM — útil enquanto o
acesso oficial não é liberado.

Suporta atualização de firmware via **ElegantOTA** (sem precisar de cabo USB).

---

## ⚠️ Primeira vez configurando o projeto (LEIA ISSO)

As credenciais (WiFi, senha do MQTT) ficam em um arquivo separado, **`src/secrets.h`**,
que **não sobe pro GitHub** (está no `.gitignore`). Isso evita vazar a senha da rede
de vocês quando o repositório for compartilhado/entregue.

**Se você acabou de clonar o repositório**, faça isso primeiro:

```bash
# Dentro da pasta do projeto
cd src
cp secrets.h.example secrets.h
```

Depois edite o `secrets.h` recém-criado com a sua rede WiFi:

```c
#define WIFI_SSID       "NOME_DA_SUA_REDE"
#define WIFI_PASSWORD   "SENHA_DA_SUA_REDE"
```

> Cada integrante do grupo cria o **seu próprio** `secrets.h` localmente.
> Por isso ele não é versionado — assim ninguém sobrescreve a rede do outro
> nem expõe senha em commit público.

---

## Requisitos de Hardware

| Componente | Detalhe |
|---|---|
| ESP32-C6 | DevKitC-1 ou equivalente |
| PZEM-004T v3.0 | Sensor de tensão/corrente/freq. (não precisa em TEST_MODE) |
| Fonte 5 V | Para alimentar o ESP32-C6 |

### Fiação UART (só necessária com `TEST_MODE = false`)

```
ESP32-C6  ←→  PZEM-004T
GPIO4 (RX) ←  TX do PZEM
GPIO5 (TX) →  RX do PZEM
GND        —  GND do PZEM
```

> **Atenção:** O PZEM-004T deve ser conectado à rede elétrica de 127/220 V pelo
> conector próprio. Siga as instruções de segurança do datasheet.

---

## Modo de Teste (sem rede da UFSM, sem sensor)

Em `src/config.h`:

```c
#define TEST_MODE true
```

Com isso o firmware:
- Gera dados simulados de tensão/corrente/frequência
- Simula ciclos de queda de energia automaticamente (pra testar o buffer)
- Usa o broker MQTT público `broker.hivemq.com` (gratuito, sem senha)

Pra ver as mensagens chegando, use o **MQTT Explorer** (app gratuito) conectando em:
- Host: `broker.hivemq.com`
- Porta: `1883`
- Usuário/senha: vazio

Tópico publicado: o valor de `MQTT_TOPIC` em `config.h` (padrão: `ufsm/pi2/grupo04/energia`).

Quando o professor liberar a rede da UFSM, troque pra `TEST_MODE false` e preencha
os dados reais em `secrets.h` (`MQTT_SERVER_REAL`, etc.).

---

## Compilar e gravar (PlatformIO)

Pela interface do VS Code: ícone do PlatformIO (formiguinha) → `esp32-c6-devkitc-1` →
**Build** (compila) → **Upload** (grava) → **Monitor** (vê os prints).

Ou pelo terminal do PlatformIO (não o PowerShell comum):

```bash
pio run                    # compila
pio run --target upload    # grava no ESP32
pio device monitor         # monitor serial
```

### Atualizações OTA (sem USB, depois da primeira gravação)

```
http://<IP_DO_ESP>/update
```

---

## Integração com InfluxDB via MQTT (modo real)

Configure o **Telegraf** pra consumir o tópico MQTT e escrever no InfluxDB:

```toml
[[inputs.mqtt_consumer]]
  servers = ["tcp://localhost:1883"]
  topics  = ["ufsm/laboratorio/energia"]
  data_format = "influx"
  precision = "s"          # IMPORTANTE: timestamp do firmware é em segundos

[[outputs.influxdb_v2]]
  urls   = ["http://localhost:8086"]
  token  = "SEU_TOKEN"
  org    = "ufsm"
  bucket = "laboratorio"
```

### Exemplo de linha gerada pelo firmware

```
sensor,sensor_id=lab_ufsm volt=219.80,amp=1.253,freq=60.0 1714406400
```

---

## Fluxo do Firmware

```
SETUP
  └─ Conectar WiFi (loop)
  └─ Sincronizar NTP
  └─ Configurar MQTT
  └─ Iniciar ElegantOTA

LOOP (a cada 3s no teste / ajustável)
  ├─ WiFi conectado? → se não: reconectar
  ├─ Ler sensor (PZEM real ou simulado)
  ├─ NTP sincronizado?
  │   ├─ Sim → salvar {volt, amp, freq, timestamp} no buffer
  │   └─ Não → descartar leitura
  ├─ MQTT conectado? → se não: conectar
  ├─ Buffer com dados?
  │   ├─ Sim + MQTT OK → enviar (Influx Line Protocol) → remover do buffer
  │   └─ Não / falha   → manter no buffer
  ├─ Atender clientes HTTP (ElegantOTA)
  └─ "Funcionando."
```

---

## Estrutura de arquivos

```
detector_energia/
├── .gitignore              ← esconde secrets.h do Git
├── platformio.ini
├── README.md
└── src/
    ├── config.h             ← configs gerais (sobe pro Git)
    ├── secrets.h.example    ← modelo de credenciais (sobe pro Git)
    ├── secrets.h            ← suas credenciais reais (NÃO sobe — gitignored)
    └── main.cpp
```

---

## Parâmetros Ajustáveis (`config.h`)

| Parâmetro | Padrão | Descrição |
|---|---|---|
| `TEST_MODE` | true | Liga/desliga simulação |
| `LOOP_INTERVAL_MS` | 3000 | Intervalo entre leituras (ms) |
| `BUFFER_MAX_SIZE` | 500 | Máx. de leituras em RAM |
| `BUFFER_SEND_PER_LOOP` | 10 | Entradas enviadas por ciclo |
| `NTP_GMT_OFFSET_SEC` | -10800 | UTC-3 (Brasília) |
| `SENSOR_UART_RX_PIN` | 4 | GPIO RX do PZEM (modo real) |
| `SENSOR_UART_TX_PIN` | 5 | GPIO TX do PZEM (modo real) |

---

## Dependências (PlatformIO)

| Biblioteca | Observação |
|---|---|
| `knolleary/PubSubClient` | Cliente MQTT |
| `ayushsharma82/ElegantOTA` | Atualização OTA |
| `mandulaj/PZEM-004T-v30` | Sensor (usado só quando `TEST_MODE=false`) |

**Platform:** usamos o fork [pioarduino](https://github.com/pioarduino/platform-espressif32)
em vez do `espressif32` padrão, porque ele tem suporte completo ao ESP32-C6 com Arduino.
