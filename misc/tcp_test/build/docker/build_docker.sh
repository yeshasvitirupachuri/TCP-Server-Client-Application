#!/bin/bash

#### Install Docker
### This is based from my own script I use at Connected Robotics Inc. to install Docker on new machines. 

## Default docker package
docker="docker-ce"

## Verify existing installation on host machine
echo " "
echo -e "\e[1;36m[installer] Verifying Docker is installed.\e[0m"
if dpkg --get-selections | grep "$docker[[:space:]]*install" ; then
    echo -e "\e[1;32m$docker installed.\e[0m"
    exit 0
else
    echo -e "\e[1;33m$docker not installed.\e[0m"
fi
    
## Install docker
echo -e " "
echo -e "\e[1;36m[installer] Downloading & installing $docker.\e[0m"
sudo apt-get -y install apt-transport-https ca-certificates curl software-properties-common
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add
sudo apt-key fingerprint 0EBFCD88
sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
sudo apt-get update
sudo apt-get -y install $docker
sudo adduser $USER docker
    
## Verify succesful installation
if dpkg --get-selections | grep "$docker[[:space:]]*install" ; then
    echo -e "\e[1;32m$docker installed.\e[0m"
    if (dialog --title " Docker installer " --yesno "\nReboot now to complete $docker installation ?\n" 7 60) then
        dialog --title " Docker installer " --infobox "\nRebooting in 5 seconds.\nRun install.sh again after reboot to complete project build.\n" 8 60
        sleep 5
        reboot
        exit 0
    else
        echo -e "\e[1;33mCancelled reboot to complete $docker installation.\e[0m"
        exit 1
    fi
else
    echo -e "\e[1;33mAn error occured and $docker has not been installed properly.\e[0m"
    exit 1
fi
