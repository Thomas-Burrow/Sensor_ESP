#!/bin/bash
set -a
source .env #obtenha token
set +a
docker compose exec influxdb influxdb3 create token --admin
