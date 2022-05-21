all: ./raspi-a/raspi-a.c ./raspi-a/gpio.c ./raspi-c/raspi-c.c
	gcc -o raspi-a_out ./raspi-a/raspi-a.c ./raspi-a/gpio.c -lpthread
	gcc -o raspi-c_out ./raspi-c/raspi-c.c -lpthread -lwiringPi
raspi-a: ./raspi-a/raspi-a.c ./raspi-a/gpio.c
	gcc -o raspi-a_out ./raspi-a/raspi-a.c ./raspi-a/gpio.c -lpthread
raspi-c: ./raspi-c/raspi-c.c
	gcc -o raspi-c_out ./raspi-c/raspi-c.c -lpthread -lwiringPi