//bibliotheken
#include <BasicStepperDriver.h>
#include <DRV8825.h>
#include <MultiDriver.h>
#include <SyncDriver.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DRV8825.h"
#include <Arduino.h>
#include <Math.h>

// stappenmotor
#define MODE0 6
#define MODE1 8 
#define MODE2 9
#define MOTOR_STEPS 200
#define DIR 2
#define STEP 4
DRV8825 stepper(MOTOR_STEPS, DIR, STEP, MODE0, MODE1, MODE2);
int steps = 0;

//LCD Callibreren
LiquidCrystal_I2C lcd(0x27, 16, 2);

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
int L = 1;    //lengte leiding
int d = 2;    //diameter leiding
float V = 0;  //gevraagde volume
float Q = 0.03;    //debiet

//berekende waardes
float Vis1 = float(1.541821619914)*pow(float(0.98017727834486),T);  //dynamische viscositeit water
float Vis2 = float(2)*pow(float(1),T);                              //dynamische viscositeit olie
float Vis = 0;
int D = 0;   //dichteheid
int P = 1013; //druk
int Pw = 0;  //drukverlies door wrijving
int Pt = 0;  //toegepaste druk (P-Pw)
int N = 0;   //kinematische viscositeit
int f = 0;   //darcy-wrijvingscoeficient
float v = 0.3; //snelheid
int Vt = 0;  //toegepaste volume

//doel
float n = 0;   //aantal omwentelingen
int t = 0;     //tijd

void setup(){
  //stappenmotor
  stepper.enable();
  
  //lcd
  lcd.init();                 //initialitatie van het LCD
  lcd.backlight();            //zet het achtergrondlicht aan
  lcd.clear();                //wist het scherm
  lcd.setCursor(0, 0);        //zet de cursor op positie 1, regel 1
  lcd.print("werkt");
  //drukknoppen
  pinMode(dk1,INPUT);
  pinMode(dk2,INPUT);
  pinMode(dk3,INPUT);
  pinMode(dk4,INPUT);
}
void loop(){
  //stepper
  stepper.setMicrostep(32); // microstepping op 1/32
  //temperatuur
  VRT = analogRead(A0);         //analoge waarde
  VRT = (5.00/1023.00)*VRT;     //spanning in V
  VR = VCC - VRT;
  RT = VRT/(VR/R);
  ln = log(RT/R0);
  T = (1/((ln/TCR) + (1/T0)));
  T = T - 273.15;               //T ==> °C

  //keuze tussen water en olie
  ss1 = digitalRead(dk1);
  ss2 = digitalRead(dk2);
  ss3 = digitalRead(dk3);
  ss4 = digitalRead(dk4);

  if (ss1 == LOW && ss2 == LOW && C == 0) {
    lcd.clear();
    lcd.print("kies een vloeistof");
    lcd.setCursor(0,1);
    lcd.print("1 = water 2 = olie");
}
  if (ss1 == HIGH && C == 0) {
    Vis = Vis1;
    D = D1;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("U koos water");
    delay(2000);
    lcd.clear();
 }
  if (ss2 == HIGH && C == 0) {
   Vis = Vis2;
   D = D2;
   lcd.clear();
   lcd.setCursor(0,0);
   lcd.print("U koos olie");
   delay(2000);
   lcd.clear();
 }
  if (ss4 = HIGH && C == 0) {
    C++;
 }
   //hoeveelheid vloeistof kiezen
  if (ss3 = HIGH && C == 1) {
   switch (C2) {
     case 0:
       V = V + 0.001;
       lcd.print(V);
       C2++;
       break;
     case 1:
       lcd.clear();
       V = V + 0.01;
       lcd.print(V);
       C2++;
       break;
     case 2:
       lcd.clear();
       V = V + 0.1;
       lcd.print(V);
       C2++;
       break;
     case 3:
       lcd.clear();
       V = V + 1;
       lcd.print(V);
       C2 = 0;
       break;
 }}
  if (ss4 = HIGH && C == 1){
    //berekening
    Re = (v * L * D)/Vis;
    if (Re < 2000){
      f = 64/Re;
      }
    else {
      lcd.clear();
      lcd.print("F error");
      delay(2000);
      lcd.clear();
    }
    Pw = f * D * (L/d) * (exp(v)/2);
    Pt = P - Pw;
    Vt = (Pt*V)/(P);
    n = Vt/Q;
    t = Vt/v;
    C++;
 }
  if (ss4 = HIGH && C == 2 && Vt != 0 && V != 0) {
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
      lcd.print("Einde");
      delay(2000);
      V = 0;
      Vt = 0;
      n = 0;
      C = 0;
      C2 = 0;
  }
 }
}
