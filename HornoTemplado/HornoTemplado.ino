#include <LiquidCrystal.h>
#include "Switch.h"
#include "config.h"
#include "PT100.h"

PT100 st(PIN_SENSOR);                      //Inicializar sensor PT100
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7); //Inicializar Display
Switch btnUp(PIN_UP, INPUT);
Switch btnDown(PIN_DOWN);
Switch btnMove(PIN_MOVE);
Switch btnSet(PIN_SET);
Switch btnOn(PIN_ON);
Switch btnError(PIN_QERROR);

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
unsigned long lastUpdateTime = 0;
bool updateNow = false;

void setup()
{
  lcd.begin(16, 2);
  lcd.createChar(1, grado);
  lcd.createChar(2, up);
  pinMode(PIN_QUEMADOR, OUTPUT);
  pinMode(PIN_RESET, OUTPUT);
}

void loop()
{
  readButtons();
  manageTemperatura();
  manageCiclo();
  updateLcd();
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
  btnUp.poll();
  if (btnUp.pushed())
  {
    lcd.setCursor(0, 10);
    lcd.print("1");
    TempladoTemp++;
    updateNow = true;
  };
  btnDown.poll();
  if (btnDown.pushed() && TempladoTemp > 10)
  {
    TempladoTemp--;
    updateNow = true;
  };
}

void updateLcd()
{
  if (((millis() - lastUpdateTime) > LCD_UPDATE_TIME)|| updateNow)
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
    if(digitalRead(PIN_UP)) lcd.write(2);
    lcd.setCursor(0, 1);
    String tiempos = "Ti=";
    tiempos.concat(String(TempladoMinActual / 60) + ":" + String(TempladoMinActual % 60));
    tiempos.concat("-");
    tiempos.concat(String(TempladoMinTotal / 60) + ":" + String(TempladoMinTotal % 60));
    tiempos.concat(" h:m");
    lcd.print(tiempos);
    lastUpdateTime = millis();
    updateNow = false;
  }
}