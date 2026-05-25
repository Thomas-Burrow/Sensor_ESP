# Compose para receber dados de um sensor com stack TICK + mosquitto


## Setup:

1. Configure os envvars em `.env` (veja `.env.example` para saber quais são usados), note que o token precisa ser gerado depois de iniciar os containers, portanto não importa nesta etapa
2. Gera pwfile do mosquitto com `gerar-mosquitto-pwfile.sh` (talvez precise de alteração de permissões com chmod/chown())
3. Inicie os containers com `docker compose up`
4. Gere token do influxdb `docker compose exec influxdb influxdb3 create token --admin --name "Admin"` e guarde ele em `.env`
5. Inicialize banco de dados com `inicializar-influxdb.sh`
6. Reinicie os containers `docker compose down;docker compose up`
