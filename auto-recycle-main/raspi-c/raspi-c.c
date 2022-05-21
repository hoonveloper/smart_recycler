#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <softPwm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define SERVO1 26
#define SERVO2 27

// Define some device parameters
#define I2C_ADDR 0x27 // I2C device address

// Define some device constants
#define LCD_CHR 1 // Mode - Sending data
#define LCD_CMD 0 // Mode - Sending command

#define LINE1 0x80 // 1st line
#define LINE2 0xC0 // 2nd line

#define LCD_BACKLIGHT 0x08 // On
// LCD_BACKLIGHT = 0x00  # Off

#define ENABLE 0b00000100 // Enable bit

void lcd_init(void);
void lcd_byte(int bits, int mode);
void lcd_toggle_enable(int bits);

// added by Lewis
void lcdLoc(int line); // move cursor
void ClrLcd(void);     // clr LCD return home
void typeln(const char *s);
void typeChar(char val);
void socket_init(const char *port);
void wiringPi_init();

int fd; // seen by all subroutines
int serv_sock;
struct sockaddr_in serv_addr, clnt_addr;
socklen_t clnt_addr_size;

void error_handling(char *message)
{
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}

int clnt_sock[2];
char save[2];

typedef enum
{
  RaspiA = 0,
  RaspiB
} Raspi;

typedef enum
{
  None = '0',
  RaspiB_SetCan = '1',
  RaspiC_RotateMotor = '1',
  RaspiB_SetPlastic = '2',
  RaspiB_TakePicture = '3',
  RaspiC_SetCapacity = 'p',
} Signal;

void lcd_print_capacity(Signal set_class, const char *capacity_text)
{
  ClrLcd();
  lcdLoc(LINE1);
  if (set_class == RaspiB_SetCan)
  {
    typeln("select Can");
  }
  else if (set_class == RaspiB_SetPlastic)
  {
    typeln("select Plastic");
  }
  lcdLoc(LINE2);
  typeln(capacity_text);
}

void *client_thread(void *arg)
{
  int input = *(int *)(arg);
  int output = !input;
  char msg[10];
  Signal set_class = None;

  char capacity_text[30];
  // printf("thead 1: %d\n",sock);
  while (1)
  {
    read(clnt_sock[input], msg, sizeof(msg));

    printf("input : %d (0 = Raspi A, 1 = Raspi B)\n", input);
    printf("Read 값 : [%s]\n", msg);

    switch (input)
    {
    case RaspiA: // Raspi A에서 받는 신호들
      switch (msg[0])
      {
      case RaspiB_SetCan:
        ClrLcd();
        lcdLoc(LINE1);
        typeln("select Can");
        set_class = RaspiB_SetCan;
        write(clnt_sock[output], msg, sizeof(msg));
        break;
      case RaspiB_SetPlastic:
        ClrLcd();
        lcdLoc(LINE1);
        typeln("select Plastic");
        set_class = RaspiB_SetPlastic;
        write(clnt_sock[output], msg, sizeof(msg));
        break;
      case RaspiB_TakePicture:
        write(clnt_sock[output], msg, sizeof(msg));
        break;
      case RaspiC_SetCapacity:
        sprintf(capacity_text, "Capacity %s%%", msg + 2);

        lcdLoc(LINE2);
        typeln(capacity_text);
        printf("percent 값 : [%c%c]\n", msg[2], msg[3]);
        break;
      }
      break;
    case RaspiB:
      switch (msg[0])
      {
      case RaspiC_RotateMotor:
        if (wiringPiSetup() == -1)
          return 0;

        softPwmCreate(SERVO1, 0, 200);

        ClrLcd();
        lcdLoc(LINE1);
        typeln("Success >o<");
        sleep(3);

        printf("모터를 작동합니다.\n");
        for (int k = 0; k < 17; k++)
        {
          softPwmWrite(SERVO1, 90);
          delay(600);
        }
        for (int k = 0; k < 8; k++)
        {
          softPwmWrite(SERVO1, 45);
          delay(600);
        }
        write(clnt_sock[output], "hey stop", sizeof("hey stop")); // Raspi A에게 모든 작업이 끝남을 알림.
        lcd_print_capacity(set_class, capacity_text);
        break;
      default:
        write(clnt_sock[output], "hey stop", sizeof("hey stop")); // Raspi A에게 모든 작업이 끝남을 알림.
        ClrLcd();
        lcdLoc(LINE1);
        typeln("Fail T_T");
        sleep(3);
        lcd_print_capacity(set_class, capacity_text);
        break;
      }
    }
  }
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Usage : %s <port>\n", argv[0]);
  }
  socket_init(argv[1]);
  wiringPi_init();
  lcd_init(); // setup LCD

  const char *PiName[] = {"Raspi A", "Raspi B"};

  printf("wait.. {Raspi A, Raspi B}\n");

  // 두개의 접속 기다림 (1번째 Raspi A, 2번째 Raspi B)
  for (int i = 0; i < 2; i++)
  {
    clnt_sock[i] = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
    if (clnt_sock[i] == -1)
      error_handling("accept() error");
    printf("%s 연결 완료\n", PiName[i]);
  }
  lcdLoc(LINE1);
  typeln("choice type");
  lcdLoc(LINE2);
  typeln("1.Can 2.Plastic");

  pthread_t handle[2];
  int sock_index[2] = {0, 1};
  pthread_create(&handle[0], NULL, client_thread, (void *)&sock_index[0]);
  sleep(2);
  pthread_create(&handle[1], NULL, client_thread, (void *)&sock_index[1]);
  sleep(2);
  pthread_join(handle[0], NULL);
  return (0);
}

// clr lcd go home loc 0x80
void ClrLcd(void)
{
  lcd_byte(0x01, LCD_CMD);
  lcd_byte(0x02, LCD_CMD);
}

// go to location on LCD
void lcdLoc(int line)
{
  lcd_byte(line, LCD_CMD);
}

// out char to LCD at current position
void typeChar(char val)
{

  lcd_byte(val, LCD_CHR);
}

// this allows use of any size string
void typeln(const char *s)
{
  while (*s)
    lcd_byte(*(s++), LCD_CHR);
}

void lcd_byte(int bits, int mode)
{

  // Send byte to data pins
  //  bits = the data
  //  mode = 1 for data, 0 for command
  int bits_high;
  int bits_low;
  // uses the two half byte writes to LCD
  bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT;
  bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT;

  // High bits
  wiringPiI2CReadReg8(fd, bits_high);
  lcd_toggle_enable(bits_high);

  // Low bits
  wiringPiI2CReadReg8(fd, bits_low);
  lcd_toggle_enable(bits_low);
}

void lcd_toggle_enable(int bits)
{
  // Toggle enable pin on LCD display
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits | ENABLE));
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits & ~ENABLE));
  delayMicroseconds(500);
}

void lcd_init()
{
  // Initialise display
  lcd_byte(0x33, LCD_CMD); // Initialise
  lcd_byte(0x32, LCD_CMD); // Initialise
  lcd_byte(0x06, LCD_CMD); // Cursor move direction
  lcd_byte(0x0C, LCD_CMD); // 0x0F On, Blink Off
  lcd_byte(0x28, LCD_CMD); // Data length, number of lines, font size
  lcd_byte(0x01, LCD_CMD); // Clear display
  delayMicroseconds(500);
}

void socket_init(const char *port)
{
  serv_sock = socket(PF_INET, SOCK_STREAM, 0);
  if (serv_sock == -1)
    error_handling("socket() error");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(atoi(port));

  if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    error_handling("bind() error");

  if (listen(serv_sock, 5) == -1)
    error_handling("listen() error");

  clnt_addr_size = sizeof(clnt_addr);
}

void wiringPi_init()
{
  if (wiringPiSetup() == -1)
    exit(1);
  fd = wiringPiI2CSetup(I2C_ADDR);
}