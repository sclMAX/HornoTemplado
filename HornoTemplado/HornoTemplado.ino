#include <LiquidCrystal.h>
#include "PT100.h"
#define PIN_QUEMADOR 3

PT100 st(A5);
LiquidCrystal lcd(7, 8, 9, 10, 11, 12); //    ( RS, EN, d4, d5, d6, d7)
byte grado[8] =
    {
        0b00001100, // Los definimos como binarios 0bxxxxxxx
        0b00010010,
        0b00010010,
        0b00001100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000};
volatile float Temp = 0;
int TempladoMinTotal = 10 * 60;
int TempladoMinActual = 0;
int TempladoTiempoInicio = 0;
int TempladoTemp = 50;
int TempEspera = 2;
bool isCiclo = true;
bool isOn = true;
bool isOnCiclo = false;

void setup()
{
  lcd.begin(16, 2);
  lcd.createChar(1, grado);
  pinMode(PIN_QUEMADOR, OUTPUT);
}

void loop()
{
  manageTemperatura();
  manageCiclo();
  readButtons();
  updateLcd();
  delay(1000);
}

void updateLcd()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  String temperaturas = "Temp:";
  temperaturas.concat(int(Temp));
  temperaturas.concat("/");
  temperaturas.concat(TempladoTemp);
  lcd.print(temperaturas);
  lcd.write(1);
  lcd.setCursor(0, 1);
  String tiempos = "Tiem:";
  tiempos.concat(TempladoMinActual);
  tiempos.concat("/");
  tiempos.concat(TempladoMinTotal);
  tiempos.concat("H");
  lcd.print(tiempos);
}

void manageTemperatura()
{
  Temp = st.getTemp();
  if (isOn)
  {
    if (Temp < (TempladoTemp - TempEspera))
      digitalWrite(PIN_QUEMADOR, HIGH);
    if (Temp > (TempladoTemp + TempEspera))
    {
      digitalWrite(PIN_QUEMADOR, LOW);
      if (!isOnCiclo && isOn && isCiclo)
      {
        isOnCiclo = true;
        TempladoTiempoInicio = millis();
      };
    };
  }
}

void manageCiclo(){
  if(isOnCiclo){
    TempladoMinActual = (millis() - TempladoTiempoInicio) / 1000 / 60;
  }
}

void readButtons()
{
}
