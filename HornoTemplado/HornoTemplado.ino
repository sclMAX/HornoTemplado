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
bool isPowerOn = false;
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
unsigned long now = 0;

void setup()
{
  lcd.begin(16, 2);
  lcd.createChar(CH_GRADO, grado);           // º
  lcd.createChar(CH_CALENTANDO, calentando); // Flecha Arriba
  lcd.createChar(CH_SELMENU, selMenu);
  lcd.createChar(CH_CICLO, ciclo);
  pinMode(PIN_QUEMADOR, OUTPUT);
  pinMode(PIN_RESET, OUTPUT);
  pinMode(PIN_POWER, OUTPUT);
  digitalWrite(PIN_QUEMADOR, LOW);
  digitalWrite(PIN_RESET, LOW);
  digitalWrite(PIN_POWER, LOW);
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
  now = millis();
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
        TempladoTiempoInicio = now;
      };
    };
  }
}

void manageCiclo()
{
  if (isOnCiclo)
  {
    TempladoMinActual = (now - TempladoTiempoInicio) / 1000 / 60;
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
  if ((((now - lastUpdateTime) > LCD_UPDATE_TIME) || updateNow) && !isPreguntaCancelar)
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
      lcd.print(temperaturas);
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
      lcd.print("T.Total " + String(tiempoProceso / 1000 / 60 / 60) + ":" + String(tiempoProceso / 1000 / 60 % 60) + "H");
    }
    else if (isFinCiclo)
    {
      lcd.setCursor(0, 0);
      lcd.print("Fin Templado");
      lcd.setCursor(0, 1);
      lcd.print("T.Total " + String(tiempoProceso / 1000 / 60 / 60) + ":" + String(tiempoProceso / 1000 / 60 % 60) + "H");
    };
    updateNow = false;
    lastUpdateTime = now;
  }
}

void readButtons()
{
  chkBtn(&btnOn, &btnOn_onClick);
  if (menu > -1) // si hay un menu seleccionado chequeo los botones
  {
    chkBtn(&btnUp, &btnUp_onClick);
    chkBtn(&btnDown, &btnDown_onClick);
  };
  chkBtn(&btnMove, &btnMove_onClick);
  chkBtn(&btnSet, &btnSet_onClick);
} // readButtons

void chkBtn(Switch *btn, void *pushed())
{
  if (btn->poll())
  {
    if (btn->pushed())
      pushed();
  }
}

void btnSet_onClick()
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
}
void btnMove_onClick()
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
}
void btnUp_onClick()
{
  if ((menu == 0) && (TempladoTemp < TEMP_MAX))
    TempladoTemp++;
  if ((menu == 1) && (TempladoMinTotal < TIEMPO_MAX))
    TempladoMinTotal++;
  updateNow = true;
}
void btnDown_onClick()
{
  if ((menu == 0) && (TempladoTemp > TEMP_MIN))
    TempladoTemp--;
  if ((menu == 1) && (TempladoMinTotal > TIEMPO_MIN))
    TempladoMinTotal--;
  updateNow = true;
}
void btnOn_onClick()
{
  if ((now - btnOnUltimoClick) > (BTNON_ESPERA_CLICK))
  {
    if (!isPowerOn)
    {
      powerOn();
    };
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
  btnOnUltimoClick = now;
}

void powerOn()
{
  digitalWrite(PIN_POWER, HIGH);
  isPowerOn = true;
}

void powerOff()
{
  if (isPowerOn)
  {
    isPowerOn = false;
    digitalWrite(PIN_POWER, LOW);
  }
}

void apagar()
{
  isOn = false;
  isCalentando = false;
  isOnCiclo = false;
  TempladoTiempoInicio = 0;
  TempladoMinActual = 0;
  digitalWrite(PIN_QUEMADOR, LOW);
  powerOff();
  tiempoProceso = now - tiempoProceso;
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