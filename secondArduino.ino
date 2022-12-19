#include<LiquidCrystal.h>
#include <string.h>


LiquidCrystal lcd(11, 12, 2, 3, 4, 5); //declarare lcd cu pinurile folosite
String mystr=""; //string pentru stocarea datelor si afisare

void setup()
{
 lcd.begin(16, 2); //initializare lcd
 Serial.begin(9600); //initializare port serial
}

void loop()
{ 
  //-------------- citire date de pe serial -----------------------
  if(Serial.available() > 0){
    int count=0; //variabila cu care verificam ca mesajul este compus din 5 carractere
    mystr=""; //variabila in care salvam mesajul

  	while(Serial.available()) // cat timp conexiunea seriala este disponibila
    {
      if(count==5) //daca am receptionat un mesaj de 5 carractere, nu mai citim
        break;
      char mychar = Serial.read();
      mystr+=mychar; //adaugarea la variabila a carracterelor pe rand
      count++;
    }
  }
  
  //-------------- procesarea si afisarea datelor -------------------
  
  //printarea textului de afisat pentru context
  lcd.setCursor(0,0);
  if(mystr[mystr.length()-2]=='0') lcd.print("Temp Optima");  //daca penultimul caracter trimis este una din variante,  
  if(mystr[mystr.length()-2]=='1') lcd.print("Temp Min");     //se afisaza textul corespunzator
  if(mystr[mystr.length()-2]=='2') lcd.print("Temp Max"); 
      

  //printarea valorii utile
  lcd.setCursor(0,1);
  if(mystr[mystr.length()-1]=='x'){ //verificarea corectitudinii receptionarii mesajului
    
    //salvarea valorii utile intr-un array de char si adaugarea virgulei
    char chr[4];
    chr[0] = mystr[0];
    chr[1] = mystr[1];
    chr[2] = ',';
    chr[3] = mystr[2];
    chr[4] = '\0';

    //afisarea pe lcd a valorii 
    lcd.print(chr);
    delay(100); // delay pentru asigurarea corectitudinii
  }
}