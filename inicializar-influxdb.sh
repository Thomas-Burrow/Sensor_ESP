#!/bin/bash
set -a
source .env #obtenha token
set +a
export TOKEN=$INFLUXDB3_AUTH_TOKEN
#podman ou docker?
podman compose exec --env INFLUXDB3_AUTH_TOKEN=$TOKEN influxdb influxdb3 create database sensor
