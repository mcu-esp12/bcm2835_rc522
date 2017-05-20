#!/bin/bash

mkdir -p /home/pi/tools
cd ./bcm2835
autoreconf -f -i
./configure --prefix=/home/pi/tools
make install
cd ..

#g++ -I /home/pi/tools/include/ -I ./ -L /home/pi/tools/lib/ main.cpp MFRC522.cpp -lbcm2835 -o mfrc522
g++ -I /home/pi/tools/include/ -I ./ -L /home/pi/tools/lib/ main.cpp MYRC522.cpp -lbcm2835 -o mfrc522

sudo ./mfrc522
