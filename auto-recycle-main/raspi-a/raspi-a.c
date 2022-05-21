#include "gpio.h"
#include <arpa/inet.h>
#include <pthread.h>

typedef int bool;
#define true 1
#define false 0

#define POUT 23
#define PIN 24
#define PWM 0

#define ul2_pin 19
#define ul2_pout 13

#define ul3_pin 2
#define ul3_pout 3

#define btn1_pin 5
#define btn1_pout 6

#define btn2_pin 27
#define btn2_pout 22

#define btn3_pin 20
#define btn3_pout 21

#define TAKE_PICTURE "3"

double distance = 0;
char msg[10];
double dist = 0;

double check_one = 0;
int percent = 0;
double check_two = 0;
int result = 0;

int sock;
int serv_sock, clnt_sock = -1;
struct sockaddr_in serv_addr, clnt_addr;
socklen_t clnt_addr_size;

int GPIO_Init();
void Client_Init(const char *ip, const char *port);

bool is_object_near(int check_cm)
{
    clock_t start_t, end_t;
    double time;

    // Enable GPIO pins
    if (-1 == GPIOExport(POUT) || -1 == GPIOExport(PIN))
    {
        printf("gpio export err\n");
        exit(0);
    }
    // wait for writing to export file
    usleep(100000);

    // Set GPIO directions
    if (-1 == GPIODirection(POUT, OUT) || -1 == GPIODirection(PIN, IN))
    {
        printf("gpio direction err\n");
        exit(0);
    }

    // init ultrawave trigger
    GPIOWrite(POUT, 0);
    usleep(10000);
    // start
    if (-1 == GPIOWrite(POUT, 1))
    {
        printf("gpio write/trigger err\n");
        exit(0);
    }
    usleep(10);
    GPIOWrite(POUT, 0);

    while (GPIORead(PIN) == 0)
    {
        start_t = clock();
    }

    while (GPIORead(PIN) == 1)
    {
        end_t = clock();
    }

    time = (double)(end_t - start_t) / CLOCKS_PER_SEC; // ms
    distance = time / 2 * 34000;

    if (distance > 900)
        distance = 900;
    printf("distance : %.2lfcm\n", distance);
    if (distance < check_cm)
    {
        result = 1;
        printf("something detected :  %lf \n ", distance);
        return true;
    }
    return false;
}

int buttonIn()
{
    printf("waiting button input...\n");
    int button_num = 0;
    if (-1 == GPIOWrite(btn3_pout, 1) || -1 == GPIOWrite(btn2_pout, 1) || -1 == GPIOWrite(btn1_pout, 1))
        return (3);
    do
    {
        if (GPIORead(btn2_pin) == 0)
        { // push button
            printf("BUTTON 2 PUSHED\n");
            button_num = 2;
            break;
        }

        if (GPIORead(btn3_pin) == 0)
        { // push button
            printf("BUTTON 3 PUSHED\n");
            button_num = 3;
            break;
        }

        if (GPIORead(btn1_pin) == 0)
        { // push button
            printf("BUTTON 1 PUSHED\n");
            button_num = 1;
            break;
        }

    } while (1);
    return button_num;
}

double check_capacity(int p_out, int p_in)
{
    clock_t start_t, end_t;
    double time;

    // Enable GPIO pins
    if (-1 == GPIOExport(p_out) || -1 == GPIOExport(p_in))
    {
        printf("gpio export err\n");
        exit(0);
    }
    usleep(100000);

    if (-1 == GPIODirection(p_out, OUT) || -1 == GPIODirection(p_in, IN))
    {
        printf("gpio direction err\n");
        exit(0);
    }

    GPIOWrite(p_out, 0);
    usleep(10000);
    // start

    if (-1 == GPIOWrite(p_out, 1))
    {
        printf("gpio write/trigger err\n");
        exit(0);
    }

    usleep(10);
    GPIOWrite(p_out, 0);
    while (GPIORead(p_in) == 0)
    {
        start_t = clock();
    }

    while (GPIORead(p_in) == 1)
    {
        end_t = clock();
    }

    time = (double)(end_t - start_t) / CLOCKS_PER_SEC; // ms
    distance = time / 2 * 34000;
    printf("용량 : %lf\n", distance);
    return distance;
}

void *init_check_capacity()
{
    printf("용량 보냄\n");
    double distance = 0;
    double distance2 = 0;
    double min_distance = 0;
    int default_distance = 40;
    int percent = 0, prev_percent = 0;
    char temp[10];
    while (1)
    {

        distance = check_capacity(ul2_pout, ul2_pin);
        // printf("distane :%lf \n",distance);
        distance2 = check_capacity(ul3_pout, ul3_pin);
        // printf("distane :%lf \n",distance2);

        min_distance = distance < distance2 ? distance : distance2;

        percent = ((default_distance - min_distance) / default_distance) * 100;
        // printf("percent: %d\n",percent);
        if (prev_percent != percent)
        {
            // 센드
            if (percent < 0)
                percent = 0;
            else if (percent > 100)
                percent = 100;
            printf("용량 percent : %d\n", percent);
            sprintf(temp, "p %02d", percent);
            write(sock, temp, sizeof(temp));
        }
        prev_percent = percent;
        sleep(5);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage : %s <ip> <port>\n", argv[0]);
        return -1;
    }
    GPIO_Init();

    Client_Init(argv[1], argv[2]);

    // /*------------메인코드-----------------*/

    int button_num = buttonIn();
    // printf("%d\n",button_num);
    sprintf(msg, "%d", button_num);
    send(sock, msg, sizeof(msg), 0); //버튼 보내기
    printf("버튼 보냄\n");

    pthread_t handle;
    pthread_create(&handle, NULL, init_check_capacity, NULL); //용량체크
    char readz[10];
    char temp[10];
    printf("대기중\n");

    // 물체가 인식되면 사진 찍으라는 신호 보냄
    while (1)
    {
        if (is_object_near(20) == true)
        {
            write(sock, TAKE_PICTURE, sizeof(TAKE_PICTURE));
            read(sock, readz, sizeof(readz)); // 다른 라즈베리파이에서 모든일이 끝나면 hey stop을 전달받음. 그때 동안 기다림.
            printf("받은 값 : %s\n", readz);
        }
        usleep(100000);
    }
    return (0);
}

int GPIO_Init()
{

    /*
     * Enable GPIO pins
     */
    if (-1 == GPIOExport(btn3_pout) || -1 == GPIOExport(btn3_pin))
        return (1);

    if (-1 == GPIOExport(btn2_pout) || -1 == GPIOExport(btn2_pin))
        return (1);
    if (-1 == GPIOExport(btn1_pout) || -1 == GPIOExport(btn1_pin))
        return (1);

    /*
     * Set GPIO directions
     */
    if (-1 == GPIODirection(btn3_pout, OUT) || -1 == GPIODirection(btn3_pin, IN))
        return (2);
    if (-1 == GPIODirection(btn2_pout, OUT) || -1 == GPIODirection(btn2_pin, IN))
        return (2);
    if (-1 == GPIODirection(btn1_pout, OUT) || -1 == GPIODirection(btn1_pin, IN))
        return (2);

    if (-1 == GPIOExport(PIN) || -1 == GPIOExport(POUT))
    {
        return 1;
    }

    if (-1 == GPIODirection(PIN, IN) || -1 == GPIODirection(POUT, OUT))
    {
        return 2;
    }

    if (-1 == GPIOWrite(POUT, 1))
    {
        return 3;
    }
}

void Client_Init(const char *ip, const char *port)
{
    sock = socket(PF_INET, SOCK_STREAM, 0);

    if (sock == -1)
    {
        error_handling("socket() error");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(atoi(port));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_handling("connect() error");
    }
}
