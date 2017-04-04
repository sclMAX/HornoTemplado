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

int eeAdress = 0;                         // Direccion actual de la memoria
int EEPROM_ver = 21;                      // Version de configuracion en memoria
int TempladoMinTotal;                     // Total de minutos para el proceso de templado
unsigned long TempladoMinActual = 0;      // Minutos transcurridos en temperatura
int TempladoTiempoInicio = 0;             // Tiempo de inicio de ciclo
int TempladoTemp;                         // Temperatura de templado
int menu = -1;                            // Menu Actual
int resetCount = 0;                       // Contador de intentos de Reset
int btnOnUltimoClick = 0;                 // Tiempo ultimo click
int errores = 0;                          // Contador de alarmas durante el proceso
bool isPowerOn = false;                   // Equipo prendido
volatile float Temp = 0;                  // Temperatura actual
volatile bool isOn = false;               // Flag en proceso
volatile bool isOnCiclo = false;          // Flag Ciclo iniciado
volatile bool isFinCiclo = false;         // Flag Ciclo terminado
volatile bool isCalentando = false;       // Flag en calentamiento
volatile bool isCancelado = false;        // Flag proceso cancelado
volatile bool isPreguntaCancelar = false; // Flag para menu Cancelar
volatile bool updateNow = false;          // Flag para actualizar pantalla al instante
volatile bool isInError = false;          //  Alarma de Error detectada
volatile bool isFalla = false;            // Falla total
unsigned long lastUpdateTime = 0;         // Tiempo ultimo refresco de pantalla
unsigned long tiempoProceso = 0;          // Contador de minutos del proceso completo
unsigned long inicioAlarma = 0;           // Hora de inicio de alarma
unsigned long oldClickTime = 0;           //Tiempo ultimo click
volatile bool oldClickState = false;      // Estado anterior del pin
unsigned long now = 0;                    // Tiempo actual

void setup()
{
  lcd.begin(16, 2);
  lcd.createChar(CH_GRADO, grado);           // º
  lcd.createChar(CH_CALENTANDO, calentando); // Flecha Arriba
  lcd.createChar(CH_SELMENU, selMenu);
  lcd.createChar(CH_CICLO, ciclo);
  lcd.createChar(CH_ALARMA, alarma);
  pinMode(PIN_QUEMADOR, OUTPUT);
  pinMode(PIN_RESET, OUTPUT);
  pinMode(PIN_POWER, OUTPUT);
  pinMode(PIN_QERROR, INPUT);
  digitalWrite(PIN_QUEMADOR, HIGH);
  digitalWrite(PIN_RESET, HIGH);
  digitalWrite(PIN_POWER, HIGH);
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
      digitalWrite(PIN_QUEMADOR, LOW);
      isCalentando = true;
    };
    if (Temp > (TempladoTemp + TEMP_ISTERESIS))
    {
      digitalWrite(PIN_QUEMADOR, HIGH);
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
    if (!isFinCiclo && !isCancelado && !isFalla)
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
      if (isInError)
        lcd.write(CH_ALARMA);
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
    }
    else if (isFalla)
    {
      lcd.setCursor(0, 0);
      lcd.print("**** FALLA ****");
      lcd.setCursor(0, 1);
      lcd.print("E" + String(errores) + "TP" + String(tiempoProceso / 1000 / 60 / 60) + ":" + String(tiempoProceso / 1000 / 60 % 60) + "TT" + String(TempladoMinActual / 60) + ":" + String(TempladoMinActual % 60));
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
  bool newClickState = digitalRead(PIN_QERROR);
  if ((newClickState != oldClickState) && ((now - 1000) >= oldClickTime))
  {
    oldClickTime = now;
    oldClickState = newClickState;
  }
  btnError_onSwitched(oldClickState);
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
          if (!isPowerOn)
          {
            powerOn();
          };
          isOn = true;
        };
      };
    }; //isOn
  };
  btnOnUltimoClick = now;
}

void btnError_onSwitched(bool estado)
{
  if (estado && isOn)
  {
    if (isInError)
    {
      reset();
    }
    else
    {
      if (inicioAlarma == 0)
      {
        isInError = true;
        errores++;
        inicioAlarma = now;
      }
    }
  }
  else
  {
    isInError = false;
    inicioAlarma = 0;
  }
}

void powerOn()
{
  digitalWrite(PIN_POWER, LOW);
  isPowerOn = true;
}

void powerOff()
{
  if (isPowerOn)
  {
    isPowerOn = false;
    digitalWrite(PIN_POWER, HIGH);
  }
}

void reset()
{
  if (resetCount < RESET_MAX)
  {
    if (now > (inicioAlarma + TIEMPO_RESET))
    {
      digitalWrite(PIN_RESET, LOW);
      delay(2000);
      digitalWrite(PIN_RESET, HIGH);
      resetCount++;
    }
  }
  else
  {
    isFalla = true;
    apagar();
  }
}

void apagar()
{
  isOn = false;
  isCalentando = false;
  isOnCiclo = false;
  TempladoTiempoInicio = 0;
  TempladoMinActual = 0;
  digitalWrite(PIN_QUEMADOR, HIGH);
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
  digitalWrite(PIN_QUEMADOR, HIGH);
  tiempoProceso = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("*** SISTEMA ***");
  lcd.setCursor(0, 1);
  lcd.print("***  LISTO  ***");
  delay(1000);
}