#!/bin/bash
set -a
source .env #obtenha usuario/senha
set +a
docker run -it --rm -v "$(pwd)/mosquitto-config:/mosquitto/config" eclipse-mosquitto sh -c "mosquitto_passwd -c -b /mosquitto/config/password_file $MQTT_USER $MQTT_PASS"
