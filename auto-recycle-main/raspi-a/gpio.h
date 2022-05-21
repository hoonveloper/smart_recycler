
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void error_handling(char *message);
int PWMExport(int pwmnum);
int PWMUnexport(int pwmnum);
int PWMEnable(int pwmnum);
int PWMUnenable(int pwmnum);
int PWMWritePeriod(int pwmnum, int value);
int PWMWriteDutyCycle(int pwmnum, int value);
int GPIOExport(int pin);
int GPIOUnexport(int pin);
int GPIODirection(int pin, int dir);
int GPIORead(int pin);
int GPIOWrite(int pin, int value);

#define BUFFER_MAX 30
#define DIRECTION_MAX 100
#define VALUE_MAX 100

#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1