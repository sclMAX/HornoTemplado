#ifndef LCDCHARS_H
#define LCDCHARS_H
//Caracteres Especiales
byte grado[8] = // GRADOS º
    {
        0b00001100, // Los definimos como binarios 0bxxxxxxx
        0b00010010,
        0b00010010,
        0b00001100,
        0b00000000,
        0b00000000,
        0b00000000,
        0b00000000};
byte calentando[8] = //CALENTANDO
    {
        0B10101,
        0B11111,
        0B00000,
        0B01010,
        0B11111,
        0B00000,
        0B10101,
        0B11111};
byte selMenu[8] = //Menu item sel.
    {
        0B00000,
        0B00001,
        0B00011,
        0B00111,
        0B01111,
        0B00111,
        0B00011,
        0B00001};
byte ciclo[8] = //En CICLO
    {
        0B01110,
        0B10001,
        0B10001,
        0B01110,
        0B10001,
        0B11001,
        0B10101,
        0B10011};
byte alarma[8] = //Alarma
    {
        0B11111,
        0B11111,
        0B11000,
        0B11111,
        0B11111,
        0B11000,
        0B11111,
        0B11111};
#define CH_GRADO 1
#define CH_CALENTANDO 2
#define CH_SELMENU 3
#define CH_CICLO 4
#define CH_ALARMA 5
#endif //LCDCHARS_H