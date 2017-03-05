#include <LiquidCrystal.h>
#include "Switch.h"
#include "config.h"
#include "lcdChars.h"
#include "PT100.h"

PT100 st(PIN_SENSOR);                      //Inicializar sensor PT100
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7); //Inicializar Display
Switch btnUp(PIN_UP, INPUT);               //btn Aumentar item
Switch btnDown(PIN_DOWN, INPUT);           //btn Disminuir item
Switch btnMove(PIN_MOVE, INPUT);           //btn Cambiar de item
Switch btnSet(PIN_SET, INPUT);             //btn Guardar configuracion
Switch btnOn(PIN_ON, INPUT);               // btn Inicio
Switch btnError(PIN_QERROR, INPUT);        // Entrada señal de ERROR de quemador

volatile float Temp = 0;
int TempladoMinTotal = 10 * 60;
int TempladoMinActual = 0;
int TempladoTiempoInicio = 0;
int TempladoTemp = 50;
int menu = -1;
bool isCiclo = true;
bool isOn = true;
bool isOnCiclo = false;
bool isCalentando = false;
unsigned long lastUpdateTime = 0;
bool updateNow = false;

void setup()
{
  lcd.begin(16, 2);
  lcd.createChar(CH_GRADO, grado);           // º
  lcd.createChar(CH_CALENTANDO, calentando); // Flecha Arriba
  lcd.createChar(CH_SELMENU, selMenu);
  lcd.createChar(CH_NADA, nada);
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

void readButtons()
{
  if (btnUp.poll())
  {
    if (btnUp.pushed())
    {
      if ((menu == 0) && (TempladoTemp < TEMP_MAX))
        TempladoTemp++;
      if ((menu == 1) && (TempladoMinTotal < TIEMPO_MAX))
        TempladoMinTotal++;
      updateNow = true;
    };
  }; //btnUp
  if (btnDown.poll())
  {
    if (btnDown.pushed())
    {
      if ((menu == 0) && (TempladoTemp > TEMP_MIN))
        TempladoTemp--;
      if ((menu == 1) && (TempladoMinTotal > TIEMPO_MIN))
        TempladoMinTotal--;
      updateNow = true;
    };
  }; //btnDown
} // readButtons

void manageTemperatura()
{
  Temp = st.getTemp();
  if (isOn)
  {
    if (Temp < (TempladoTemp - TEMP_ISTERESIS))
    {
      digitalWrite(PIN_QUEMADOR, HIGH);
      isCalentando = true;
    };
    if (Temp > (TempladoTemp + TEMP_ISTERESIS))
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

void updateLcd()
{
  if (((millis() - lastUpdateTime) > LCD_UPDATE_TIME) || updateNow)
  {
    lcd.clear();
    if (menu > -1)
    {
      lcd.setCursor(15, menu);
      lcd.write(CH_SELMENU);
    };

    lcd.setCursor(0, 0);
    String temperaturas = "Te=";
    temperaturas.concat(int(Temp));
    temperaturas.concat("/");
    temperaturas.concat(TempladoTemp);
    lcd.print(temperaturas);
    lcd.write(CH_GRADO);
    if (isCalentando)
    {
      lcd.write(CH_CALENTANDO);
    }
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