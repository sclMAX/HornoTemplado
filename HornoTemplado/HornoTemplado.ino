#include <LiquidCrystal.h>
#include <EEPROM.h>
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

int eeAdress = 0;
int EEPROM_ver = 21;
int TempladoMinTotal;
int TempladoMinActual = 0;
int TempladoTiempoInicio = 0;
int TempladoTemp;
int menu = -1;
int btnOnUltimoClick = 0;
volatile float Temp = 0;
volatile bool isOn = false;
volatile bool isOnCiclo = false;
volatile bool isFinCiclo = false;
volatile bool isCalentando = false;
volatile bool isCancelado = false;
volatile bool isPreguntaCancelar = false;
volatile bool updateNow = false;
unsigned long lastUpdateTime = 0;
unsigned long tiempoProceso = 0;

void setup()
{
  lcd.begin(16, 2);
  lcd.createChar(CH_GRADO, grado);           // º
  lcd.createChar(CH_CALENTANDO, calentando); // Flecha Arriba
  lcd.createChar(CH_SELMENU, selMenu);
  lcd.createChar(CH_CICLO, ciclo);
  pinMode(PIN_QUEMADOR, OUTPUT);
  pinMode(PIN_RESET, OUTPUT);
  readConfig();
}

void readConfig()
{
  int version = 0;
  EEPROM.get(eeAdress, version);
  if (version == EEPROM_ver)
  {
    eeAdress += sizeof(int);
    EEPROM.get(eeAdress, TempladoMinTotal);
    eeAdress += sizeof(int);
    if (eeAdress >= EEPROM.length())
      eeAdress = 0;
    EEPROM.get(eeAdress, TempladoTemp);
  }
  else
  {
    TempladoMinTotal = 2;
    TempladoTemp = 50;
    saveConfig();
  }
}

void saveConfig()
{
  eeAdress = 0;
  EEPROM.put(eeAdress, EEPROM_ver);
  eeAdress += sizeof(int);
  if (eeAdress >= EEPROM.length())
    eeAdress = 0;
  EEPROM.put(eeAdress, TempladoMinTotal);
  eeAdress += sizeof(int);
  if (eeAdress >= EEPROM.length())
    eeAdress = 0;
  EEPROM.put(eeAdress, TempladoTemp);
}

void loop()
{
  manageTemperatura();
  manageCiclo();
  updateLcd();
  readButtons();
}

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
      if (!isOnCiclo && isOn)
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
    if (TempladoMinActual >= TempladoMinTotal)
    {
      isFinCiclo = true;
      isOnCiclo = false;
      apagar();
    };
  };
}

void updateLcd()
{
  if ((((millis() - lastUpdateTime) > LCD_UPDATE_TIME) || updateNow) && !isPreguntaCancelar)
  {
    lcd.clear();
    if (!isFinCiclo && !isCancelado)
    {
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
      lcd.print(temperaturas + " ");
      lcd.write(CH_GRADO);
      if (isCalentando && isOn)
        lcd.write(CH_CALENTANDO);
      if (isOn)
        lcd.write(CH_CICLO);
      lcd.setCursor(0, 1);
      String tiempos = "Ti=";
      tiempos.concat(String(TempladoMinActual / 60) + ":" + String(TempladoMinActual % 60));
      tiempos.concat("-");
      tiempos.concat(String(TempladoMinTotal / 60) + ":" + String(TempladoMinTotal % 60));
      lcd.print(tiempos);
    }
    else if (isCancelado)
    {
      lcd.setCursor(0, 0);
      lcd.print("** CANCELADO **");
      lcd.setCursor(0, 1);
      lcd.print("T.Total:" + String(tiempoProceso / 1000 / 60 / 60) + ":" + String(tiempoProceso / 1000 / 60 % 60));
    }
    else if (isFinCiclo)
    {
      lcd.setCursor(0, 0);
      lcd.print("Fin Templado");
      lcd.setCursor(0, 1);
      lcd.print("T.Total:" + String(tiempoProceso / 1000 / 60 / 60) + ":" + String(tiempoProceso / 1000 / 60 % 60));
    };
    updateNow = false;
    lastUpdateTime = millis();
  }
}

void readButtons()
{
  if (btnOn.poll())
  {
    if (btnOn.pushed())
    {
      if ((millis() - btnOnUltimoClick) > (BTNON_ESPERA_CLICK))
      {
        if (isOn)
        {
          LCD_Cancelar();
        }
        else // !isOn
        {
          if (isCancelado)
          {
            preparar();
          }
          else
          {
            if (isFinCiclo)
            {
              isFinCiclo = false;
              preparar();
            }
            else
            {
              preparar();
              isOn = true;
            };
          };
        }; //isOn
      };
      btnOnUltimoClick = millis();
    };           //btnOn.pushed()
  };             //btnOn
  if (menu > -1) // si hay un menu seleccionado chequeo los botones
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
  };
  if (btnMove.poll())
  {
    if (btnMove.pushed())
    {
      if (isPreguntaCancelar)
      {
        isPreguntaCancelar = false;
      }
      else
      {
        menu++;
        if (menu > 1)
          menu = -1;
      };
      updateNow = true;
    };
  }; //btnMove
  if (btnSet.poll())
  {
    if (btnSet.pushed())
    {
      if (isPreguntaCancelar && isOn)
      {
        isPreguntaCancelar = false;
        isCancelado = true;
        apagar();
      }
      else if (menu > -1)
      {
        saveConfig();
        menu = -1;
        updateNow = true;
      };
    };
  }; //btnSet
} // readButtons

void apagar()
{
  isOn = false;
  isCalentando = false;
  isOnCiclo = false;
  TempladoTiempoInicio = 0;
  TempladoMinActual = 0;
  digitalWrite(PIN_QUEMADOR, LOW);
  tiempoProceso = millis() - tiempoProceso;
}
void LCD_Cancelar()
{
  if (!isPreguntaCancelar)
  {
    isPreguntaCancelar = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SET: CANCELAR...");
    lcd.setCursor(0, 1);
    lcd.print("MOVE: ATRAS...");
  };
}
void preparar()
{
  isFinCiclo = false;
  isCancelado = false;
  isOn = false;
  isCalentando = false;
  isOnCiclo = false;
  isPreguntaCancelar = false;
  TempladoTiempoInicio = 0;
  TempladoMinActual = 0;
  digitalWrite(PIN_QUEMADOR, LOW);
  tiempoProceso = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("*** SISTEMA ***");
  lcd.setCursor(0, 1);
  lcd.print("***  LISTO  ***");
  delay(1000);
}