#include <EEPROM.h>
#define address 0

struct DATOS
{
    float temperatura;
    float humedad;
    int luz;
    boolean usable;
};

union MEMORIA {
    DATOS dato;
    byte b[sizeof(DATOS)];
} miMemoria;

void readMem()
{
    // Se recuperan los datos de la memoria EEPROM:
    for (int i = 0; i < sizeof(DATOS); i++)
        miMemoria.b[i] = EEPROM.read(address + i);

    // Se comprueba que se hayan podido leer los datos (que no se haya leido basura)
    // No es la forma mas elegante de hacerlo pero funciona:
    if (miMemoria.dato.usable == true)
    {
        miMemoria.dato.temperatura += 0.5;
        miMemoria.dato.humedad += 0.1;
        miMemoria.dato.luz += 1;
    }
    // Si nunca se habian usado se inicializan todos los datos:
    else
    {
        miMemoria.dato.temperatura = 0.0;
        miMemoria.dato.humedad = 0.0;
        miMemoria.dato.luz = 0;
        miMemoria.dato.usable = true;
    }
}

void saveMem()
{
    // Se guardan los datos en la memoria EEPROM:
    for (int i = 0; i < sizeof(DATOS); i++)
        EEPROM.write(address + i, miMemoria.b[i]);
}