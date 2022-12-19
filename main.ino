//DECLARARE LIBRARII
//#include <Wire.h>

//##############################PWM
void ConfigPWM()
{
DDRD |= (1<<DDD6)|(1<<DDD5);//data direction register
TCCR0A = 0b10100011; 
TCCR0B = 0b11000001;// 62500 Hz
}
void ClearPWMSetting()
{
TCCR0A = 0b00000011; //valoarea de fabrica
TCCR0B = 0b00000011; //valoarea de fabrica
TCNT0 = 0b11100100; //valoarea de fabrica
}

void writePWM(int port, int value)
{
  if(port == 5)
  OCR0A = value;
  if(port == 6)
  OCR0B = value;
}
//##############################PWM
//DECLARARE FUNCTII
void activeTempControl(float);
void passiveTempControl();
void lightControl(int);
void openWindow();
void closeWindow();
bool useOutdoor(float ,float);
bool checkHeating();
bool checkCooling();

void sendOverWire(float);
void pushButton();

//DECLARARE VARIABILE GLOBALE
float tempOpt = 21; //TEMPERATURA OPTIMA SETATA DE UTILIZATOR FOLOSITA IN PREZENTA ACESTUIA
float tempTarget = 0; //TEMPERATURA PE CARE SISTEMUL INCEARCA SA O ATINGA
float tempMin = 18; //TEMPERATURA MINIMA SETATA DE UTILIZATOR
float tempMax = 23; //TEMPERATURA MAXIMA SETATA DE UTILIZATOR
float maxDifference = 0.5; //DIFERENTA DINTRE TEMPERATURA OPTIMA SI CEA REALA MAXIMA
int sensorState = 0; //STAREA SENZORULUI DE PREZENTA
unsigned long startTime;
bool movement; //VARIABILA CARE INDICA PREZENTA (DETECTATA DE PIR)
bool previousState; //STAREA ANTERIOARA A SISTEMULUI DE ILUMINAT
bool lightSetting=0; //SETARE CARE INDICA CE FACE SISTEMUL DE ILUMINAT IN CAZUT IN CARE NU ESTE PREZENT UTILIZATORUL (0-sist. se stinge total<---->1-sist. functioneaza la luminozitatea minima)
bool window = 0; //VARIABILA CARE RETINE STAREA GEAMULUI (0-inchis<---->1-deschis)
int buttonState = 0;//STAREA INITIALA A BUTONULUI
int previousButtonState = 0;//STAREA ANTERIOARA A BUTONULUI
int tempSelect = 1;
int potetiometerValue = 0;
int outputValue = 0;
int prevOutputValue = 0;

#define pirSensor 13
#define ledHeating 4
#define ledCooling 7
#define outTempSensor A0
#define inTempSensor A1
#define motorDriver1 5
#define motorDriver2 6
#define ledLightSist 3
#define BUTTON_PIN 2
#define POTENTIOMETER_PIN A2

void setup()
{
  pinMode(pirSensor, INPUT);//Senzor miscare
  pinMode(ledHeating, OUTPUT);//Led Incalzire
  pinMode(ledCooling, OUTPUT);//Led Racire
  pinMode(outTempSensor, INPUT);//Senzor afara
  pinMode(inTempSensor, INPUT);//Senzor inauntru
  pinMode(motorDriver1, OUTPUT);//Driver Motor
  pinMode(motorDriver2, OUTPUT);//Driver Motor
  pinMode(ledLightSist, OUTPUT);//Bec
  pinMode(BUTTON_PIN, INPUT_PULLUP);//PushButton
  pinMode(A2, INPUT);
  ConfigPWM();
  //Wire.begin();
  Serial.begin(9600);
  //Serial.begin(76800);
}

void loop() {
  
    
  sensorState = digitalRead(pirSensor);
  //Serial.println(sensorState);
  delay(5000);
  buttonState = digitalRead(BUTTON_PIN);
  potetiometerValue = analogRead(A2);
  
  if(previousButtonState == 1 && buttonState == 0)
  {
    pushButton();
  }
  
  outputValue = map(potetiometerValue, 0, 1023, 20, 60); //setarea valorii citite intre 10 si 30 de grade cu o sensibilitate de 0.5 grade
  
	if(sensorState == HIGH)
 		movement=0;
	else
    movement=1;
  //SE VERIFICA IN CE STARE AR TREBUI SA SE AFLE SISTEMUL DE ILUMINAT
  lightControl(movement);
  //CITIRE TEMPERATURA DE LA SENZORI
	int tempOut=map(((analogRead(A0) - 20) * 3.04), 0, 1023, -40, 125);
	int tempIn=map(((analogRead(A1) - 20) * 3.04), 0, 1023, -40, 125);
  
	float tempDifference; //DIFERENTA DINTRE TEMPERATURA INTERIOARA SI CEA DORITA
	if (movement) {
		tempTarget = tempOpt;
		tempDifference = tempIn - tempTarget;
		if (tempDifference > maxDifference || tempDifference < maxDifference * (-1)) { //VERIFICA DACA DIFERENTA DE TEMPERATURA ESTE SEMNIFICATIVA
			if (useOutdoor(tempDifference,tempOut))
				passiveTempControl();
			else
				activeTempControl(tempDifference);
		}
	}
	else {
		if (tempIn <= tempMin) {
			tempTarget = tempMin + 1;
			tempDifference = tempIn - tempTarget;
			activeTempControl(tempDifference);
		}
		else {
			if (tempIn >= tempMax) {
				tempTarget = tempMax - 1;
				tempDifference = tempIn - tempTarget;
				activeTempControl(tempDifference);
			}
		}
	}
  
  // daca s-a modificat valoarea potentiometrului sau a butonului
  if(prevOutputValue!=outputValue||previousButtonState != buttonState){ 

    char charToSend[5]; // variabila care salveaza valuarea temperaturii ce trebuie trimisa
                        // si carracterele necesare pentru afisarea corecta

    //trei cazuri in functie de starea setata de buton
    if(tempSelect == 1) {
      tempOpt = float(outputValue) / 2; // impartirea valorii de la potentiometru pentru a obtine valoarea de afisat
      itoa(int(tempOpt*100),charToSend,10); //convertirea de la float->int->char*
      charToSend[4]='x'; // adaugarea caracterului x pentru detectia corecta a valorii de afisat
      Serial.write(charToSend); // trimiterea sirului
      delay(600); //delay pentru asigurarea corectitudinii transmisiei
    }
    if(tempSelect == 2) {
      tempMin = float(outputValue) / 2;
      itoa(int(tempMin*100+1),charToSend,10);
      charToSend[4]='x';
      Serial.write(charToSend);
      delay(600);
    }
    if(tempSelect == 3) {
      tempMax = float(outputValue) / 2;    
      itoa(int(tempMax*100+2),charToSend,10); 
      charToSend[4]='x';
      Serial.write(charToSend);
      delay(600);
    }
    prevOutputValue = outputValue; // salvarea valorii potentiometrului
   }
  
  previousButtonState = buttonState; // salvarea valorii butonului
}

//SE SETEAZA STAREA CORECTA A SISTEMULUI DE ILUMINAT
void lightControl(int movement){
  //Serial.println("light control");
  unsigned long currentTime = millis(); //TIMPUL DE CAND FUNCTIONEAZA SISTEMUL
  if(movement == 0){
      if(previousState==1)
      {
       	 previousState=0;
         startTime = millis();
      }
      if(currentTime - startTime > 300000){  //VERIFICA DACA A TRECUT 'N' TIMP (1000 = 1sec)
      	if(lightSetting==0){
          digitalWrite(ledLightSist, LOW);
          //Serial.println("light shut down");
          }
      	else
          digitalWrite(ledLightSist, HIGH);
      }
    }
  	else if (movement == 1){
      digitalWrite(ledLightSist, HIGH);
      previousState=1;
    }
}

//ACTIUNI IN CAZUL IN CARE SE FOLOSESTE SIST. DE INCALZIRE/RACIRE
void activeTempControl(float tempDifference){
  if (window == 1){
    //SE INCHIDE GEAMUL
		closeWindow();
  }
	if (tempDifference > 0) {
		if (checkHeating())
			//SE INCHIDE SIST. DE INCALZIRE
      digitalWrite(ledHeating, LOW);
		if (!checkCooling())
			//SE PORNESTE SIST. DE RACIRE
      digitalWrite(ledCooling,HIGH);
	}
	else if (tempDifference < 0) {
		if (checkCooling())
			//SE INCHIDE SIST. DE RACIRE
      digitalWrite(ledCooling, LOW);
		if (!checkHeating())
			//SE DESCHIDE SISTEMUL DE INCALZIRE
      digitalWrite(ledHeating, HIGH);
	}
}

//ACTIUNI IN CAZUL IN CARE SE FOLOSESTE TEMPERATURA EXTERIOARA
void passiveTempControl() {
	if (checkHeating())
    //SE INCHIDE SIST. DE INCALZIRE
  	digitalWrite(ledHeating, LOW);
	if (checkCooling())
		//SE INCHIDE SIST. DE RACIRE
 		digitalWrite(ledCooling, LOW);
  if (window == 0){
    //SE DESCHIDE GEAMUL
    openWindow();
  }
}

//VERIFICA DACA SE POATE FOLOSII TEMPERATURA EXTERIOARA
bool useOutdoor(float tempDifference, float tempOut) {
	if (tempDifference > 1 && tempOut <=tempTarget)
		  return true;
	else if (tempDifference < -1 && tempOut >= tempTarget)
		  return true;
	return false;
}

//VERIFICA STAREA SISTEMULUI DE INCALZIRE
bool checkHeating() {
	bool heating;
  if(digitalRead(ledHeating)==HIGH)
  	return 1;
  else
    return 0;
}

//VERIFICA STAREA SISTEMULUI DE RACIRE
bool checkCooling() {
	bool cooling;
  if(digitalRead(ledCooling)==HIGH)
  	return 1;
  else
    return 0;
}

void pushButton(){
  if(tempSelect== 3)
    tempSelect = 1;
  else
    tempSelect++;
  Serial.print(tempSelect);
  
}



//SE DESCHIDE GEAMUL
void openWindow(){
 window = 1;
 /* digitalWrite(motorDriver2, HIGH);
  digitalWrite(motorDriver1, LOW);
  delay(1000);
  digitalWrite(motorDriver2, LOW);
  digitalWrite(motorDriver1, LOW);*/
  writePWM(motorDriver2, 150);
  writePWM(motorDriver1, 0); 
  delay(50000);
  writePWM(motorDriver2, 0);
  writePWM(motorDriver1, 0);
}

void closeWindow(){
  window = 0;
  /*digitalWrite(motorDriver2, LOW);
  digitalWrite(motorDriver1, HIGH);*/
  //delay(1000);
  writePWM(motorDriver2, 0);
  writePWM(motorDriver1, 150); 
  delay(50000);
  writePWM(motorDriver2, 0);
  writePWM(motorDriver1, 0);
 /* digitalWrite(motorDriver2, LOW);
   digitalWrite(motorDriver1, LOW);*/
}

void sendOverWire(float value){
//Wire.beginTransmission(9);
//Wire.write(int(value));
Serial.write(int(value));
//Wire.endTransmission();
}
