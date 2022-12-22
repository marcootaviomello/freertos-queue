
/*
Task A should print any new messages it receives from Queue 2. 
Additionally, it should read any Serial input from the user and echo back 
this input to the serial input. If the user enters “delay” followed by a space and a number, 
it should send that number to Queue 1.

Task B should read any messages from Queue 1. 
If it contains a number, it should update its delay rate to that number (milliseconds). 
It should also blink an LED at a rate specified by that delay. 
Additionally, every time the LED blinks 100 times, it should send the string “Blinked” to Queue 2. 
You can also optionally send the number of times the LED blinked (e.g. 100) as part of struct that 
encapsulates the string and this number.
*/

/* Macros to use*/
#define ZeroMemory(Dest) memset(Dest, 0, sizeof(Dest))
#define CopyMemory(Dest, Orig) memcpy(Dest, Orig, sizeof(Orig))
#define CopyString(Dest, Orig) memcpy(Dest, Orig, strlen(Orig) + 1)
#define PrintArraySerial(Array, LastElement) for(int x=0; x<LastElement; x++) {Serial.print(Array[x]);} Serial.println();

/*CONSTANTES*/
#define MAX_ELEMENTS_QUEUE 10
#define BUFFER_LEN 255
#define CMD_PREFIX "delay "

//static const char command[] = "delay "; // Note the space!

/* Include libs from FreeRTOS */
#include  "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/* Criando  a estrutura sugerida no ultimo paragrafo que tera o cabecalho
Blinked: + contador*/
typedef struct Message {
    char body[20];
    int contador;
} Message;

/*Cria os gerenciadores para as Queues propostas */
QueueHandle_t h_queue1; /*vai armazenar o tempo do delay*/
QueueHandle_t h_queue2; /*Vai armazenar a string blinked*/
TaskHandle_t h_taskA; 

void taskA (void *parameter)
{
    Message msg_rcvd;
    char buf_serial[BUFFER_LEN];
    char buf_delay_value[5];
    uint8_t indice = 0;
    uint8_t aux_control = 0;
    char c;

    ZeroMemory(buf_serial);
    ZeroMemory(buf_delay_value);

    for(;;)
    {
        /*Verifica se tem msg na Queue 2, se tiver imprime.*/
        if (xQueueReceive(h_queue2, (void *)&msg_rcvd, 0) == pdTRUE) 
        {
            Serial.print("[TASK A]: TEM DADO NA QUEUE 2 >> ");
            Serial.println(msg_rcvd.body);
            Serial.println(msg_rcvd.contador);
        }

        /*Verifica se tem dados da serial*/
        if (Serial.available() > 0)
        {
            c = Serial.read();

            /*verifica se não vai estourar o buffer*/
            if (indice < (BUFFER_LEN-1))
            {
                buf_serial[indice] = c;
                indice++;
            }

            /*Verififica se o caractere atual é o caractere de fim de transmissao
            neste caso verifica um \r ou \n */
            if ((c == '\n') || (c == '\r'))
            {
                Serial.println("[TASK A]: Terminou de receber dados.");

                /*Valida os dados para ver se o que foi recebido é uma string de configuração que inicia
                com 'delay ' */
                if (memcmp(buf_serial, CMD_PREFIX, 6) == 0)
                {
                    Serial.println("Recebeu a string de configuracao.");

                    int j = 0;

                    while (aux_control == 0)
                    {
                        char digit = buf_serial[6+j];
                        if (isdigit(digit) &&  j<5)
                        {
                            buf_delay_value[j] = digit;
                            j++;
                        }
                        else
                        {
                           aux_control = 1; 
                        }
                    }

                    Serial.print("DELAY TIME >>>> ");
                    Serial.println(buf_delay_value);

                    aux_control = 0;
                    ZeroMemory(buf_serial);
                    ZeroMemory(buf_delay_value);
                    indice = 0;

                }else
                {
                    PrintArraySerial(buf_serial, indice);
                    ZeroMemory(buf_serial);
                    indice = 0;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}


void setup()
{
    h_queue1 = xQueueCreate( MAX_ELEMENTS_QUEUE, sizeof( int ));
	h_queue2 = xQueueCreate( MAX_ELEMENTS_QUEUE, sizeof( Message ));

    /* Verifica de algumas das Queues foi não foi criada e está NULL*/
    configASSERT(h_queue1);
    configASSERT(h_queue2);

    Serial.begin(115200);
    xTaskCreatePinnedToCore(taskA, "TASK_A", 1024, NULL, 1, &h_taskA, APP_CPU_NUM);

    String str_teste = "teste queue";

    // if(xQueueSend(h_queue2, (void *) &str_teste, 10) != pdTRUE)
    // {
    //     Serial.println("Queue full");
    // }

}

void loop()
{
	
}
