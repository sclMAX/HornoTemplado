#include <LiquidCrystal.h>
#include "PT100.h"

PT100 st(A5);
LiquidCrystal lcd(7, 8, 9, 10, 11, 12); //    ( RS, EN, d4, d5, d6, d7)
byte grado[8] =
 {
    0b00001100,     // Los definimos como binarios 0bxxxxxxx
    0b00010010,
    0b00010010,
    0b00001100,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
 };
 volatile float Temp = 0;
 volatile int Dato = 0;

void setup() {
  lcd.begin(16,2);
  lcd.createChar(1, grado);
}

void loop() {
  Temp = st.getTemp();
  Dato = st.getDato();
  lcd.setCursor(0,0);
  lcd.print("Dato: ");
  lcd.print(Dato);
  lcd.setCursor(0,1);
  lcd.print("Temp: ");
  lcd.setCursor(6,1);
  lcd.print(Temp);
  lcd.write(1);
  delay(1000);
}
