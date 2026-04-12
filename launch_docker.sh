docker build -t plansys2-monitoring .

docker run -d --network=host --privileged \
    --name plansys2-monitoring plansys2-monitoring \
    sleep infinity