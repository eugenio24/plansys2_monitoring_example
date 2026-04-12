#!/bin/bash

if [ "$(docker ps -aq -f name=plansys2-monitoring)" ]; then
    echo "Container already present, starting if stopped..."
    docker start plansys2-monitoring 2>/dev/null
    echo "Attach with: docker exec -it plansys2-monitoring bash"
    exit 0
fi

docker build -t plansys2-monitoring .

docker run -d --network=host --privileged \
    --hostname plansys2-monitoring-docker  \
    --name plansys2-monitoring plansys2-monitoring \
    sleep infinity

echo "Container ready. Attach with: docker exec -it plansys2-monitoring bash"