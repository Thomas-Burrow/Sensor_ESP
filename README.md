# Compose para receber dados de um sensor com stack TICK + mosquitto


## Setup:
1. Configure os envvars em `.env` (veja `.env.example` para saber quais são usados), note que o token precisa ser gerado então pode aguardar
2. Inicie os containers com `podman-compose up`
3. Gere token do influxdb `podman-compose exec influxdb influxdb3 create token --admin --name "Admin"` e guarde ele em `.env`
4. Gera pwfile do mosquitto com `gerar-mosquitto-pwfile.sh`
5. Inicialize banco de dados com `inicializar-influxdb.sh`
6. Reinicie os containers `podman-compose up;podman-compose down`