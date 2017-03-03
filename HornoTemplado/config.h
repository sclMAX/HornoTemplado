#ifndef CONFIG_H
#define CONFIG_H
#define LCD_UPDATE_TIME 1000 //Retardo de actualizacion de LCD
#define PIN_UP 4             // Menu SET +
#define PIN_DOWN 3           // Menu SET -
#define PIN_MOVE 2           // Menu NEXT
#define PIN_SET 1            // Guardar opciones
#define PIN_QUEMADOR 6       // Quemador On/Off
#define PIN_RESET 5          // Quemador RESET
#define PIN_ON 0             // On/Off ciclo
#define PIN_QERROR 13        // Alarma Quemador
#define PIN_SENSOR A5        // Sensor de temperatura
#define RS 7                 // LCD RS
#define EN 8                 // LCD EN                  
#define D4 9                 // LCD D4
#define D5 10                // LCD D5
#define D6 11                // LCD D6
#define D7 12                // LCD D7
typedef struct
{
    int pin = -1;
    bool estadoAct = false;
    bool estadoAnt = false;
} botones_type;
#define CANT_BOTONES 6
volatile botones_type botones[CANT_BOTONES];

void setUpBotones()
{

    botones[0].pin = PIN_UP;
    botones[1].pin = PIN_DOWN;
    botones[2].pin = PIN_MOVE;
    botones[3].pin = PIN_SET;
    botones[4].pin = PIN_ON;
    botones[5].pin = PIN_QERROR;
    for (int i = 0; i < CANT_BOTONES; i++)
    {
        pinMode(botones[i].pin, INPUT);
    };
}

#endif // !CONFIG_H