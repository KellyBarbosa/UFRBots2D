#!/bin/bash

clear

echo "Script para montagem do ambiente de simulação 2D."
echo -e "Será instalado: rcssserver-18.1.2, rcssmonitor-18.0.0 e librcsc-4.1.0.\n\n"

echo "Instalando as dependências do simulador..."
sleep 2

sudo apt update

sudo apt install -y build-essential automake autoconf libtool flex bison libboost-all-dev libfontconfig1-dev libaudio-dev libxt-dev libglib2.0-dev libxi-dev libxrender-dev qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools libfl-dev

clear
echo "Baixando os arquivos do simulador..."
sleep 2
cd ~
sudo rm -rf rcss
mkdir rcss
cd rcss
wget https://github.com/rcsoccersim/rcssserver/releases/download/rcssserver-18.1.2/rcssserver-18.1.2.tar.gz
wget https://github.com/rcsoccersim/rcssmonitor/releases/download/rcssmonitor-18.0.0/rcssmonitor-18.0.0.tar.gz

clear
echo "Baixando a librcsc..."
sleep 2
wget https://pt.osdn.net/projects/rctools/downloads/51941/librcsc-4.1.0.tar.gz

clear
echo "Instalando o ambiente de simulação. Esta operação pode demorar alguns minutos..."
sleep 5
tar xzvfp rcssserver-18.1.2.tar.gz
cd rcssserver-18.1.2
sudo ./configure
sudo make install
cd ..
tar xzvfp rcssmonitor-18.0.0.tar.gz
cd rcssmonitor-18.0.0
sudo ./configure
sudo make install
cd ..

clear
echo "Instalando a librcsc. Esta operação pode demorar alguns minutos..."
sleep 5
tar xzvfp librcsc-4.1.0.tar.gz
cd librcsc-4.1.0
sudo ./configure
sudo make install

clear

echo "Instalação finalizada!"

rm -rf rcss
