#!/bin/bash
set -a
source .env #obtenha token
set +a
docker compose exec --env  influxdb influxdb3 create token --admin