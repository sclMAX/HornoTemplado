#ifndef CONFIG_H
#define CONFIG_H
#define LCD_UPDATE_TIME 1000      // Retardo de actualizacion de LCD
#define BTNON_ESPERA_CLICK 2000      //Espera entre pulsaciones btnOn
#define TEMP_MIN 50               // Temperatura Minima
#define TEMP_MAX 250              // Temperatura Maxima
#define TEMP_ISTERESIS 2          // Margen en ºC para prender/apagar quemador
#define TIEMPO_MIN 1              // Tiempo Minimo en minutos
#define TIEMPO_MAX 900            // Tiempo Maximo en minutos

//**** Configuracion de PINES ****
#define PIN_UP 4       // Menu SET +
#define PIN_DOWN 3     // Menu SET -
#define PIN_MOVE 2     // Menu NEXT
#define PIN_SET 1      // Guardar opciones
#define PIN_QUEMADOR 6 // Quemador On/Off
#define PIN_RESET 5    // Quemador RESET
#define PIN_ON 13      // On/Off ciclo
#define PIN_QERROR 0   // Alarma Quemador
#define PIN_SENSOR A5  // Sensor de temperatura
#define RS 7           // LCD RS
#define EN 8           // LCD EN
#define D4 9           // LCD D4
#define D5 10          // LCD D5
#define D6 11          // LCD D6
#define D7 12          // LCD D7
//**** Configuracion de PINES ****

#endif // !CONFIG_H