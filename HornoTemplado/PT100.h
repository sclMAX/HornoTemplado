#ifndef PT100_O
#include "Arduino.h"
#define PT100_O
class PT100
{
  private:
    static const int sensorDatoLow = 537; //Dato leido a -Cº.
    static const int sensorDatoDif = 35;  //Diferencia entre Dato leido a -Cº y +Cº.572@59º - 537@9º
    static const int tempValueLow = 9;    //Valor real de -Cº medido.
    static const int tempValueDif = 50;    //Diferencia entre valor medido a -Cº y +Cº.59º - 9º
    int PIN;
    double Temp;
    float dato;
    void readTemp();

  public:
    PT100(int);
    float getTemp();
    float getDato();

};

PT100::PT100(int pin)
{
    PIN = pin;
    dato = 0;
    Temp = 0;
};

void PT100::readTemp()
{
    dato = analogRead(PIN);
    Temp = dato - sensorDatoLow;
    Temp = Temp / sensorDatoDif;
    Temp = Temp * tempValueDif;
    Temp = Temp + tempValueLow;
};

float PT100::getTemp()
{
    readTemp();
    return Temp;
};

float PT100::getDato()
{
    return dato;
};
#endif // !PT100_O