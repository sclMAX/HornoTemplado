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
byte up[8] =
    {
        0B00100,
        0B01110,
        0B11111,
        0B00100,
        0B00100,
        0B00100,
        0B00100,
        0B00000};
volatile float Temp = 0;
int TempladoMinTotal = 10 * 60;
int TempladoMinActual = 0;
int TempladoTiempoInicio = 0;
int TempladoTemp = 50;
int TempEspera = 2;
bool isCiclo = true;
bool isOn = true;
bool isOnCiclo = false;
bool isCalentando = false;

void setup()
{
  lcd.begin(16, 2);
  lcd.createChar(1, grado);
  lcd.createChar(2, up);
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
  String temperaturas = "Te=";
  temperaturas.concat(int(Temp));
  temperaturas.concat("/");
  temperaturas.concat(TempladoTemp);
  lcd.print(temperaturas);
  lcd.write(1);
  if (isCalentando)
  {
    lcd.write(2);
  }
  lcd.setCursor(0, 1);
  String tiempos = "Ti=";
  tiempos.concat(String(TempladoMinActual / 60) + ":" + String(TempladoMinActual % 60));
  tiempos.concat("-");
  tiempos.concat(String(TempladoMinTotal / 60) + ":" + String(TempladoMinTotal % 60));
  tiempos.concat(" h:m");
  lcd.print(tiempos);
}

void manageTemperatura()
{
  Temp = st.getTemp();
  if (isOn)
  {
    if (Temp < (TempladoTemp - TempEspera))
    {
      digitalWrite(PIN_QUEMADOR, HIGH);
      isCalentando = true;
    };
    if (Temp > (TempladoTemp + TempEspera))
    {
      digitalWrite(PIN_QUEMADOR, LOW);
      isCalentando = false;
      if (!isOnCiclo && isOn && isCiclo)
      {
        isOnCiclo = true;
        TempladoTiempoInicio = millis();
      };
    };
  }
}

void manageCiclo()
{
  if (isOnCiclo)
  {
    TempladoMinActual = (millis() - TempladoTiempoInicio) / 1000 / 60;
  }
}

void readButtons()
{
}
