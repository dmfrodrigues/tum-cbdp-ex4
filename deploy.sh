#!/bin/sh

if [ $# -eq 0 ] || [ $1 -gt 5 ]
then
  echo "Usage: ./deploy.sh num_workers(max 5 workers)"
  exit 1
fi

export $(grep -v '^#' .env | xargs)
docker build -t "$REGISTRY_NAME.azurecr.io"/coordinator --target coordinator .
docker build -t "$REGISTRY_NAME.azurecr.io"/worker --target worker .
docker push "$REGISTRY_NAME.azurecr.io"/coordinator
docker push "$REGISTRY_NAME.azurecr.io"/worker

az container start -g cbdp-resourcegroup --name coordinator
COORDINATOR=$(az container show --resource-group cbdp-resourcegroup --name coordinator --query ipAddress.ip --output tsv)
for (( i=1; i<=$1; i++ ))
do
  az container start -g cbdp-resourcegroup --name worker$i
done