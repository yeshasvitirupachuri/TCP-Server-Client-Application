#!/bin/bash

# Open an interative bash instance in the docker container
#   open_container.sh

# Execute <commands> in the docker container
#   open_container.sh "<command1>; <command2>..."

if [[ $# -ge 1 ]] ; then
    echo -e "\e[33mExecuting command(s) in bash instance on docker container:\e[0m"
    echo -e "\e[33m$1\e[0m"
    docker exec -t agilerobots.tcp_test.$USER bash -ci "$1"
else
    echo -e "\e[33mOpening interactive bash instance on docker container:\e[0m"
    docker exec -i -t agilerobots.tcp_test.$USER bash
fi
echo -e "\e[33mDocker container bash instance terminated.\e[0m"




