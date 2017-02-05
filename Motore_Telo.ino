#include <EEPROM.h>
#include "Arduino.h"

//Variabili e costanti
int FcAlto;           //pin finecorsa
int enable = 10;            //pin motore
int IN1 = 12;               //output direzione motore
int IN2 = 11;               //output direzione motore
int comandoGiu;
int comandoSu;
bool flagFC = false;        //indica se il finecorsa � stato utilizzato
int posizione = 1125;
int ris;                    //risultato per il calcolo delle velocità di salita e discesa
bool isDown = false;

//pulsanti
//------------------------------------------------------------------
int buttonStateGiu = 0;
int buttonStateSu = 0;
int buttonStateVeloce = 0;
int su = 8;  //input
int giu = 13; //input
int veloce = 9;  //input
int memorizza;
//-------------------------------------------------------------------

//variabili encoder
//-------------------------------------------------------------------
int encoderPin1 = 2;
int encoderPin2 = 3;

volatile long lastEncoded = 0;
volatile long encoderValue = 0;

long lastencoderValue = 0;

long lastMSB = 0;
long lastLSB = 0;

int addr = 0; //indirizzo nella EEPROM
			  //--------------------------------------------------------------------

void setup() {
	// put your setup code here, to run once:
	Serial.begin(9600);
	pinMode(6, INPUT);
	pinMode(su, INPUT);
	pinMode(giu, INPUT);
	pinMode(veloce, INPUT);
	pinMode(memorizza, INPUT);
	pinMode(enable, OUTPUT);
	pinMode(IN1, OUTPUT);
	pinMode(IN2, OUTPUT);
	pinMode(14, INPUT);
	pinMode(5, INPUT);
	pinMode(4, OUTPUT);

	pinMode(encoderPin1, INPUT);
	pinMode(encoderPin2, INPUT);

	attachInterrupt(0, updateEncoder, CHANGE);
	attachInterrupt(1, updateEncoder, CHANGE);
	//addressInt = EEPROM.getAddress(sizeof(int));
}

void loop() {
	// put your main code here, to run repeatedly:

	//Riporta il telo su fino al finecorsa
	FcAlto = digitalRead(14);

	if (flagFC == false) {
		//if(digitalRead(memorizza)==HIGH){}
		if (FcAlto == LOW) {

			analogWrite(enable, 0);
			flagFC = true;
			encoderValue = 0;
		}
		else {

			digitalWrite(IN1, HIGH);
			digitalWrite(IN2, LOW);
			analogWrite(enable, 75);
			FcAlto = digitalRead(14);

		}
	}

	controlloAutomatico();
}

void controlloAutomatico() {
	//set di istruzioni per la discesa
	if (flagFC == true) {

		comandoGiu = digitalRead(6);
		Serial.println("Valore encoder: ");
		Serial.println(encoderValue);
		if (comandoGiu == LOW) {

			ris = posizione - encoderValue;         //calcolo per la posizione se il valore è sopra il 10
			if (ris > 50) {                         //viene usata la velocità massima, se arriva tra 10 e 1

				digitalWrite(IN1, LOW);               //abbassa la velocità, se arriva a 0 si ferma
				digitalWrite(IN2, HIGH);
				analogWrite(enable, 255);
				ris = posizione - encoderValue;
				Serial.println("Giu Veloce\n");
				Serial.println("ris: ");
				Serial.println(ris);

			}
			else if (ris <= 50) {

				if (ris <= 0) {

					analogWrite(enable, 0);
					Serial.println("motore fermo");
					Serial.println("ris: ");
					Serial.println(ris);

				}
				else {

					digitalWrite(IN1, LOW);
					digitalWrite(IN2, HIGH);
					analogWrite(enable, 50);
					Serial.println("Giu lento");
					Serial.println("ris: ");
					Serial.println(ris);

				}
			}

			comandoGiu = digitalRead(6);

		}
		else {

			analogWrite(enable, 0);

		}
	}

	if (flagFC == true) {

		comandoSu = digitalRead(5);
		if (comandoSu == LOW) {

			FcAlto = digitalRead(14);
			if (FcAlto == HIGH) {                       //Codice di salita (non funziona)

				if (encoderValue > 50) {

					digitalWrite(IN1, HIGH);
					digitalWrite(IN2, LOW);                 //Salita veloce
					analogWrite(enable, 255);

				}
				else if (encoderValue <= 50) {

					digitalWrite(IN1, HIGH);
					digitalWrite(IN2, LOW);
					analogWrite(enable, 75);

				}
			}
			else {

				analogWrite(enable, 0);
				digitalWrite(4, HIGH);                    //attiva il rientro del telo nel muro

			}
		}
	}
}

void controlloManuale() {

	statoBottoni();
	if (buttonStateGiu == LOW) {

		statoBottoni();
		while (buttonStateGiu == LOW) {

			if (buttonStateGiu == LOW && buttonStateVeloce == LOW) {

				digitalWrite(IN1, LOW);
				digitalWrite(IN2, HIGH);
				analogWrite(enable, 255);
				statoBottoni();

			}
			else {

				digitalWrite(IN1, LOW);
				digitalWrite(IN2, HIGH);
				analogWrite(enable, 80);
				statoBottoni();

			}
		}
	}
	else {

		analogWrite(enable, 0);
	}

	statoBottoni();
	if (buttonStateSu == LOW) {

		statoBottoni();
		while (buttonStateSu == LOW) {

			if (buttonStateSu == LOW && buttonStateVeloce == LOW) {

				digitalWrite(IN1, HIGH);
				digitalWrite(IN2, LOW);
				analogWrite(enable, 255);
				statoBottoni();

			}
			else {

				digitalWrite(IN1, HIGH);
				digitalWrite(IN2, LOW);
				analogWrite(enable, 100);
				statoBottoni();

			}
		}
	}
	else {

		analogWrite(enable, 0);
	}
}

void statoBottoni() {

	buttonStateGiu = digitalRead(giu);
	buttonStateSu = digitalRead(su);
	buttonStateVeloce = digitalRead(veloce);

}

void updateEncoder() {
	long MSB = digitalRead(encoderPin1); //MSB = most significant bit
	long LSB = digitalRead(encoderPin2); //LSB = least significant bit

	long encoded = (MSB << 1) | LSB; //converting the 2 pin value to single number
	long sum = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

	if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue++;
	if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue--;

	lastEncoded = encoded; //store this value for next time
}

void writeEEPROM(long encoderValue) {

	int eeAdd = 0;								//indirizzo nella eeprom

	EEPROM.put(eeAdd, encoderValue);
}

int readEEPROM() {

	int eeAdd = 0;
	long pos;

	EEPROM.get(eeAdd, pos);
	return pos;
}
