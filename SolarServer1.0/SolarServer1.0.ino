/****************************************************** 
* ESP32 Analog Input Test 
* Analog Input: ADC_1_0 pin ==> GPIO36 (VP).
* 
* MJRoBot.org 6Sept17
*****************************************************/
//Analog Input
#define ADC1_CHANNEL_6 35 // GPIO 35
int analog_value = 0;

#include <ESP32Servo.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>



Servo myservo;  // create servo object to control a servo
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // pin remapping with ESP8266 HW I2C


int pos = 0;    // variable to store the servo position
// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33 
int servoPin = 18;

// assume 4x6 font, define width and height
#define U8LOG_WIDTH 32
#define U8LOG_HEIGHT 2

// allocate memory
uint8_t u8log_buffer[U8LOG_WIDTH*U8LOG_HEIGHT];
// Create a U8g2log object
U8G2LOG u8g2log;

int mVperAmp = 185; 
int RawValue = 0;
int ACSoffset = 2.5;
double Voltage = 0;
double Amps = 0;


void setup_display(void) {
  u8g2.begin();    
  u8g2.setFont( u8g2_font_t0_11_mf );  // set the font for the terminal window
  u8g2log.begin(u8g2, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer); // connect to u8g2, assign buffer
  u8g2log.setLineHeightOffset(0); // set extra space between lines in pixel, this can be negative
  u8g2log.setRedrawMode(0);   // 0: Update screen with newline, 1: Update screen for every char 
  u8g2log.print("Mqtt Client...");
  u8g2log.print("\n");
}

void setup()
{
  Serial.begin(115200);

  setup_display();

  myservo.setPeriodHertz(50);    // standard 50 hz servo
	myservo.attach(servoPin, 1000, 2000); 
  delay(1000); // give me times to bring up serial monitor
  Serial.println("ESP32 Analog IN Test");

  u8g2log.print("SolarServer");u8g2log.print("\n");


}

void Calcula_corrente()
{
  RawValue = analogRead(ADC1_CHANNEL_6);
  Voltage = RawValue * ( 3.3 / 4095.0);
  
  Amps = ((Voltage - ACSoffset) / mVperAmp);


  Serial.printf("%4d\ %4.2f\ %4.2f\n", RawValue, Voltage, Amps); 

}

void loop()
{
  Calcula_corrente();
  delay(200);

//for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
//		// in steps of 1 degree
//		myservo.write(pos);    // tell servo to go to position in variable 'pos'
//		delay(15);             // waits 15ms for the servo to reach the position
//	}
//	for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
//		myservo.write(pos);    // tell servo to go to position in variable 'pos'
//		delay(15);             // waits 15ms for the servo to reach the position
//	}

}
