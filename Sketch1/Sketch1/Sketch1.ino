
// Inductance and Capacitance code was gathered by a website. This code is meant to combine the 2 meters into a single meter 
// that alternates between each with a button press.
#include <Wire.h>
#include <LiquidCrystal.h>

//LCD
LiquidCrystal lcd(1, 2, 4, 5, 6, 7);

//button switch setup
int state = HIGH;
int reading;
int previous = LOW;
long time = 0;
long debounce = 200;
int button_low = 3;
int button_high = 12;

//Capacitance
int ScalepF = 8;
int ScalenF = 10;
//High values//
#define analogPin      A0          
#define chargePin      5         
#define dischargePin   6       
#define resistorValue  10000.0F  //Remember, we've used a 10K resistor to charge the capacitor
unsigned long startTime;
unsigned long elapsedTime;
float microFarads;
float nanoFarads;
//Low values//
const int OUT_PIN = A2;
const int IN_PIN = A0;
const float IN_STRAY_CAP_TO_GND = 50.28; //We have to change te resistance in this configuration. The 10K and 220 resistors are changing the values									
const float IN_CAP_TO_GND = IN_STRAY_CAP_TO_GND;
const float R_PULLUP = 30.0;
const int MAX_ADC_VALUE = 1023;

//Inductance
//13 is the input to the circuit (connects to 150ohm resistor), 11 is the comparator/op-amp output.
double pulse, frequency, capacitance, inductance;

void setup() {
	//button 
	pinMode(button_low, INPUT);
	pinMode(button_high, OUTPUT);

	//LCD
	lcd.begin(20, 4);

	//Capacitance
	pinMode(ScalepF, INPUT);
	pinMode(ScalenF, INPUT);
	pinMode(OUT_PIN, OUTPUT);
	pinMode(IN_PIN, OUTPUT);
	pinMode(chargePin, OUTPUT);

	//Inductance code
	Serial.begin(115200);
	pinMode(11, INPUT);
	pinMode(13, OUTPUT);
	delay(200);
}
void loop() {
	//Button
	reading = digitalRead(button_low);
	//Button if statement starts
	if (reading == HIGH && previous == LOW && millis() - time > debounce) {
		if (state == HIGH) {
			//Inductance
			digitalWrite(13, HIGH);
			delay(5);//give some time to charge inductor.
			digitalWrite(13, LOW);
			delayMicroseconds(100); //make sure resination is measured
			pulse = pulseIn(11, HIGH, 5000);//returns 0 if timeout
			if (pulse > 0.1) { //if a timeout did not occur and it took a reading:

		   //   #error insert your used capacitance value here. Currently using 2uF. Delete this line after that
				capacitance = 2.E-6; // - insert value here

				frequency = 1.E6 / (2 * pulse);
				inductance = 1. / (capacitance*frequency*frequency*4.*3.14159*3.14159);//one of my profs told me just do squares like this
				inductance *= 1E6; //note that this is the same as saying inductance = inductance*1E6

				//Serial print
				Serial.print("High for uS:");
				Serial.print(pulse);
				Serial.print("\tfrequency Hz:");
				Serial.print(frequency);
				Serial.print("\tinductance uH:");
				Serial.println(inductance);
				delay(10);

				lcd.clear();
				lcd.setCursor(0, 0);
				lcd.print("Inductance:");
				lcd.setCursor(0, 1);
				lcd.print(inductance);
				lcd.setCursor(14, 1);
				lcd.print("uH");
				delay(10);
			}

			state = LOW;
		}
		else
			//Capacitance
				//////////////////////////nF/////////////////////////////////////////
			if (digitalRead(ScalenF)) {
				pinMode(OUT_PIN, OUTPUT);
				digitalWrite(OUT_PIN, LOW); //to make it GND
				pinMode(analogPin, INPUT); //This pin will read the voltage
				digitalWrite(chargePin, HIGH);
				startTime = micros();
				while (analogRead(analogPin) < 648) {
				}
				elapsedTime = micros() - startTime;
				microFarads = ((float)elapsedTime / resistorValue);
				if (microFarads > 1) {
					lcd.clear();
					lcd.setCursor(0, 0);
					lcd.print("SCALE:  100nF-4F");
					lcd.setCursor(0, 1);
					lcd.print(microFarads);
					lcd.setCursor(14, 1);
					lcd.print("uF");
					delay(500);
				}
				else {
					nanoFarads = microFarads * 1000.0;
					lcd.clear();
					lcd.setCursor(0, 0);
					lcd.print("SCALE:  100nF-4F");
					lcd.setCursor(0, 1);
					lcd.print(nanoFarads);
					lcd.setCursor(14, 1);
					lcd.print("nF");
					delay(500);
				}
				digitalWrite(chargePin, LOW);
				pinMode(dischargePin, OUTPUT);
				digitalWrite(dischargePin, LOW);     //discharging the capacitor     
				while (analogRead(analogPin) > 0) {
				}//This while waits till the capaccitor is discharged
				pinMode(dischargePin, INPUT);      //this sets the pin to high impedance
				lcd.setCursor(0, 0);
				lcd.print("DISCHARGING.....");
				lcd.setCursor(0, 1);
			}
		//////////////////////////pF/////////////////////////////////////////
		if (digitalRead(ScalepF)) {
			pinMode(chargePin, INPUT);
			pinMode(dischargePin, INPUT);//We give high impedance to the two pins. We don't use this pins    
			pinMode(IN_PIN, INPUT);
			digitalWrite(OUT_PIN, HIGH);
			int val = analogRead(IN_PIN);
			digitalWrite(OUT_PIN, LOW);

			if (val < 976) {
				pinMode(IN_PIN, OUTPUT);

				float capacitance = (float)val * IN_CAP_TO_GND / (float)(MAX_ADC_VALUE - val);
				lcd.clear();
				lcd.setCursor(0, 0);
				lcd.print("Scale:   1pF-1nF");
				lcd.setCursor(0, 1);
				lcd.print(capacitance, 3);
				lcd.setCursor(14, 1);
				lcd.print("pF");
				delay(200);
			}
			else {
				pinMode(IN_PIN, OUTPUT);
				delay(1);
				pinMode(OUT_PIN, INPUT_PULLUP);
				unsigned long u1 = micros();
				unsigned long t;
				int digVal;

				do {
					digVal = digitalRead(OUT_PIN);
					unsigned long u2 = micros();
					t = u2 > u1 ? u2 - u1 : u1 - u2;
				} while ((digVal < 1) && (t < 400000L));

				pinMode(OUT_PIN, INPUT);
				val = analogRead(OUT_PIN);
				digitalWrite(IN_PIN, HIGH);
				int dischargeTime = (int)(t / 1000L) * 5;
				delay(dischargeTime);
				pinMode(OUT_PIN, OUTPUT);
				digitalWrite(OUT_PIN, LOW);
				digitalWrite(IN_PIN, LOW);

				float capacitance = -(float)t / R_PULLUP / log(1.0 - (float)val / (float)MAX_ADC_VALUE);

				lcd.setCursor(0, 0);
				lcd.print("Scale:   1pF-1nF");

				if (capacitance > 1000.0) {
					lcd.clear();
					lcd.setCursor(0, 0);
					lcd.print("Scale:   1pF-1nF");
					lcd.setCursor(0, 1);
					lcd.print(capacitance / 1000.0, 3);
					lcd.setCursor(14, 1);
					lcd.print("uF ");
					delay(200);

				}
				else {
					lcd.clear();
					lcd.setCursor(0, 0);
					lcd.print("Scale:   1pF-1nF");
					lcd.setCursor(0, 1);
					lcd.print(capacitance, 3);
					lcd.setCursor(14, 1);
					lcd.print("nF");
					delay(200);
				}
			}
			while (micros() % 1000 != 0);
		}
		state = HIGH;
	}
}
