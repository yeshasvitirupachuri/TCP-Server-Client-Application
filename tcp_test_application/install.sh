#!/bin/bash

set -e

## Install Docker app
chmod +x docker/build_docker.sh
docker/build_docker.sh

## Clean existing docker image & container for this project
if ! [ -e docker/Dockerfile ] ; then
    echo -e "\e[1;31mmissing docker/Dockerfile\e[0m"
    exit 1
fi
if docker container ls -a | grep "agilerobots.tcp_test.$USER"; then
    echo -e "\e[1;33mRemoving old docker container.\e[0m"
    docker stop agilerobots.tcp_test.$USER
    docker rm agilerobots.tcp_test.$USER
fi
if docker images | grep "agilerobots/tcp_test"; then
    echo -e "\e[1;33mRemoving old docker image.\e[0m"
    docker rmi agilerobots/tcp_test
fi

## Build docker image
echo -e "\e[1;33mBuilding docker image.\e[0m"
docker build --build-arg username=$USER --build-arg uid=$UID --build-arg gid=$GROUPS -f docker/Dockerfile -t agilerobots/tcp_test .

## Build docker container
## the project is mounted to the container to facilitate development
echo -e "\e[1;33mBuilding docker container.\e[0m"
APP_NAME=tcp_test_application
docker run -ti -d -v $(dirname $(readlink -f "$0")):/src/${APP_NAME} --name agilerobots.tcp_test.$USER agilerobots/tcp_test

## Build project in docker container
echo -e "\e[1;33mBuilding cmake project.\e[0m"
docker/open_container.sh "echo \"welcome to the container!\"; sleep 2"
docker exec -w "/src/${APP_NAME}" -t agilerobots.tcp_test.$USER /bin/bash -ci "ls && mkdir -p build"
docker exec -w "/src/${APP_NAME}/build" -t agilerobots.tcp_test.$USER /bin/bash -ci "mkdir -p install && echo $(pwd)"
docker exec -w "/src/${APP_NAME}/build" -t agilerobots.tcp_test.$USER /bin/bash -ci "ls && cmake ../ -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/src/${APP_NAME}/build/install"
docker exec -w "/src/${APP_NAME}/build" -t agilerobots.tcp_test.$USER /bin/bash -ci "make all && make install"

## Build complete: stop container
docker stop agilerobots.tcp_test.$USER
echo -e "\e[1;32mBuild complete.\e[0m"
