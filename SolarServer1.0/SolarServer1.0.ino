/****************************************************** 
* Solar Server 1.0 
* Analog Input: 
* 
* MJRoBot.org 6Sept17
*****************************************************/
int analog_value = 0;

#include <ESP32Servo.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include <ESP32analogReadNonBlocking.h>


		ESP32analogReadNonBlocking ADC_pin25(25, 100000);
    ESP32analogReadNonBlocking ADC_pin26(26, 100000);
    


	uint8_t ArbitrationToken1; //Use one Aribtration Token for anything on ADC1, GPIO: 34, 35, 36, 37, 38, 39ADC_
	uint8_t ArbitrationToken2; //Use one Arbitration Token for anything on ADC2, GPIO: 4, 12, 13, 14, 15, 25, 26, 27

	uint32_t loopcounter;//loop counter for example to show that the code is non-blocking
	uint32_t loopcounterSerialPrintTimer;



Servo myservo;  // create servo object to control a servo

//U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // pin remapping with ESP8266 HW I2C
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // ESP32 Thing, HW I2C with pin remapping

int pos = 0;    // variable to store the servo position
// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33 
int servoPin = 18;

// assume 4x6 font, define width and height
#define U8LOG_WIDTH 64
#define U8LOG_HEIGHT 4

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
  u8g2log.begin(u8g2, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer); // connect to u8g2, assign buffer
  u8g2log.setLineHeightOffset(0); // set extra space between lines in pixel, this can be negative
  u8g2log.setRedrawMode(0);   // 0: Update screen with newline, 1: Update screen for every char 
  u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
  u8g2.drawGlyph(1, 20, 69);	
  delay(2000);
  u8g2.setFont( u8g2_font_6x12_tr);  // set the font for the terminal window
  u8g2log.print("Solar Server...");
  u8g2log.print("\n");
}

void setup()
{
  Serial.begin(115200);

  setup_display();

  myservo.setPeriodHertz(50);    // standard 50 hz servo
	myservo.attach(servoPin, 1000, 2000); 
  delay(1000); // give me times to bring up serial monitor
  
  u8g2log.print("SolarServer");u8g2log.print("\n");


}

void Calcula_corrente()
{
  //RawValue = analogRead(ADC1_CHANNEL_6);
  Voltage = RawValue * ( 3.3 / 4095.0);
  
  Amps = ((Voltage - ACSoffset) / mVperAmp);


  Serial.printf("%4d\ %4.2f\ %4.2f\n", RawValue, Voltage, Amps); 

}

double GetVoltage(double reading){
   // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
  if(reading < 1 || reading > 4095) return 0;
  // return -0.000000000009824 * pow(reading,3) + 0.000000016557283 * pow(reading,2) + 0.000854596860691 * reading + 0.065440348345433;
  return -0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089;
} 


void loop()
{

ADC_pin25.tick(ArbitrationToken1);
ADC_pin26.tick(ArbitrationToken2);



	if (ADC_pin25.newValueFlag) { 
	                          Serial.print("GPIO25 raw counts = "); 
	                          Serial.print(ADC_pin25.counts); 
                            Voltage = ADC_pin25.counts * ( 3.3 / 4095.0);
                            Serial.print(" Voltage: "); 
                            Serial.print(Voltage); 
                            Serial.print(" Voltage new: "); 
                            Voltage =  GetVoltage(ADC_pin25.counts);
                            Serial.println(Voltage); 
                            u8g2log.print("Raw ADC0:");u8g2log.print(ADC_pin25.counts);u8g2log.print("\n");
                            u8g2log.print("Raw ADC1:");u8g2log.print(ADC_pin26.counts);u8g2log.print("\n");
                            u8g2log.print("Voltage:");u8g2log.print(Voltage);u8g2log.print("\n");
                            //Calcula_corrente();
	                         }

  



//		myservo.write(pos);    // tell servo to go to position in variable 'pos'


}
