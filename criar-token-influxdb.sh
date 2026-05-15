#!/bin/bash
set -a
source .env #obtenha token
set +a
#podman ou docker?
podman compose exec --env  influxdb influxdb3 create token --admin