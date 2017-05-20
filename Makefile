bcm2835_build:
	mkdir -p /home/pi/tools
	cd ./bcm2835
	./configure --prefix=/home/pi/tools
	make install
	cd ..

all: bcm2835_build
	g++ -I /home/pi/tools/include/ -I ./ -L /home/pi/tools/lib/ main.cpp MFRC522.cpp -lbcm2835 -o mfrc522
	sudo ./mfrc522

my: bcm2835_build
	g++ -I /home/pi/tools/include/ -I ./ -L /home/pi/tools/lib/ main.cpp MYRC522.cpp -lbcm2835 -o mfrc522
	sudo ./mfrc522
