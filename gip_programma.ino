//bibliotheken
#include <SevSeg.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DRV8825.h"
#include <Arduino.h>

// stappenmotor
#define MODE0 10
#define MODE1 11 
#define MODE2 12
#define MOTOR_STEPS 200
#define DIR 8
#define STEP 9
DRV8825 stepper(MOTOR_STEPS, DIR, STEP, SLEEP, MODE0, MODE1, MODE2);
stepper.setMicrostep(32); // microstepping op 1/32
int steps = 0;

//LCD Callibreren
LiquidCrystal_I2C lcd(0x27, 20, 4);

//drukknoppen
  //dk1
int dk1 = 2;
int ss1 = LOW;       //schakelstand dk1
  //dk2
int dk2 = 4;
int ss2 = LOW;       //schakelstand dk2
  //dk3
int dk3 = 6;
int ss3 = LOW;       //schakelstand dk3
  //dk4
int dk4 = 8;
int ss4 = LOW;       //schakelstand dk4
//NTC
  //zie datasheet
int R0 = 100;      //weerstand bij 0°C
int TCR = 3850;    //temperatuurcoëfficient in K
float T0 = 25 + 273.15;  //temperatuur in K bij 0 ohm 
  //gegeven
int VCC = 5;       //klemspanning in V
int R = 10000;     //begrens weerstand 
  //variabelen
float RT = 0;   //weerstand van PTC
float VR = 0;   //spanning over R
float ln = 0;   //logaritme van (Rt/R0)
float VRT = 0;  //TV zender
double T = 0;    //temperatuur
//counter
int C = 0;
int C2 = 0;
//gegeven waardes
int D1 = 997; //dichtheid water
int D2 = 918; //dichtheid olie
int Re = 0;   //Reinoldsgetal
int ov = 0;   //opstartsnelheid motor
int mv = 0;   //maximale snelheid motor
int L = 1;    //lengte leiding
int d = 2;    //diameter leiding
float V = 0;        //gevraagde volume

//berekende waardes
float Vis1 = float(1.541821619914)*pow(float(0.98017727834486),T);  //dynamische viscositeit water
float Vis2 = float(2)*pow(float(1),T);                              //dynamische viscositeit olie
float Vis = 0;
int D = 0;   //dichteheid
int Pw = 0;  //drukverlies door wrijving
int Pt = 0;  //toegepaste druk (P-Pw)
int N = 0;   //kinematische viscositeit
int f = 0;   //darcy-wrijvingscoeficient
int v = 0;   //snelheid

//doel
float n = 0;   //aantal omwentelingen
int t = 0;   //tijd

void setup(){
  //stappenmotor
  stepper.enable();
  
  //lcd
  lcd.init();                 //initialitatie van het LCD
  lcd.backlight();            //zet het achtergrondlicht aan
  lcd.clear();                //wist het scherm
  lcd.setCursor(0, 0);        //zet de cursor op positie 1, regel 1
  
  //drukknoppen
  pinMode(dk1,INPUT);
  pinMode(dk2,INPUT);
  pinMode(dk3,INPUT);
  pinMode(dk4,INPUT);
}
void loop(){
  //temperatuur
  VRT = analogRead(A0);         //analoge waarde
  VRT = (5.00/1023.00)*VRT;     //spanning in V
  VR = VCC - VRT;
  RT = VRT/(VR/R);
  ln = log(RT/R0);
  T = (1/((ln/TCR) + (1/T0)));
  T = T - 273.15;               //T ==> °C

  //timer
  lcd.print(t);     //t = tijd
  t = t-1;          // waarde van t daalt met 1 
  delay(950);       //delay van 0.95 seconden
  lcd.clear();      //lcd word leeggemaakt

  //keuze tussen water en olie
  ss1 = digitalRead(dk1);
  ss2 = digitalRead(dk2);
  ss3 = digitalRead(dk3);
  ss4 = digitalRead(dk4);

  if (ss1 == LOW && ss2 == LOW && C == 0) {
    lcd.clear();
    lcd.write("kies een vloeistof");
    lcd.setCursor(0,1);
    lcd.write("1 = water 2 = olie");
}
  if (ss1 == HIGH && C == 0) {
    C++;
    Vis = Vis1;
    D = D1;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.write("U koos water");
    delay(2000);
    lcd.clear();
 }
  if (ss2 == HIGH && C == 0) {
   C++;
   Vis = Vis2;
   D = D2;
   lcd.clear();
   lcd.setCursor(0,0);
   lcd.write("U koos olie");
   delay(2000);
   lcd.clear();
 }
   //hoeveelheid vloeistof kiezen
  if (ss3 = HIGH && C == 1) {
   switch (C2) {
     case 0:
       V = V + 0.001;
       lcd.write(V);
       C2++;
       break;
     case 1:
       lcd.clear();
       V = V + 0.01;
       lcd.write(V);
       C2++
       break;
     case 2:
       lcd.clear();
       V = V + 0.1;
       lcd.write(V);
       C2++;
       break;
     case 3:
       lcd.clear();
       V = V + 1;
       lcd.write(V);
       C2 = 0;
 }}
  if (ss4 = HIGH && C == 1){
    //berekening
    C++;
 }
  if (ss4 = HIGH && C == 2 && V != 0) {
    //motor draait 'steps' aantal stappen
    int(steps) = n * 6400;
    stepper.move(steps);
    //timer
    lcd.clear();      //lcd word leeggemaakt
    lcd.print(t);     //t = tijd
    t = t-1;          // waarde van t daalt met 1 
    delay(950);       //delay van 0.95 seconden
    if (t == 0 && C == 2) {
      delay(1000);
      lcd.clear();
      lcd.write("Einde");
      delay(2000);
      V = 0;
      n = 0;
      C = 0;
      C2 = 0;
  }
 }
}
