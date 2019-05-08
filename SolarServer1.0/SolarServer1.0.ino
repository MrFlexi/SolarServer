

/******************************************************
  Solar Server 1.0
  Analog Input:

  MJRoBot.org 6Sept17
*****************************************************/
#include <Arduino.h>
#include <Wire.h>

//--------------------------------------------------------------------------
// Wifi Setup + MQTT 
//--------------------------------------------------------------------------
#include <WiFi.h>
#include <PubSubClient.h>
const char* ssid = "MrFlexi";
const char* password = "Linde-123";
const char* mqtt_server = "192.168.1.144";   // Laptop
//const char* mqtt_server = "test.mosquitto.org";   // Laptop

const char* topic = "mrflexi/solarserver/";




//--------------------------------------------------------------------------
// JSON Setup 
//--------------------------------------------------------------------------
#include <ArduinoJson.h>    // https://github.com/bblanchon/ArduinoJson
DynamicJsonDocument doc(1024);
char msg[200];

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsgAlive = 0;
long lastMsgDist = 0;

//--------------------------------------------------------------------------
// Analog Read Setup  ADC
//--------------------------------------------------------------------------
#include <ESP32analogReadNonBlocking.h>

int ADC25 = 25;
int analog_value = 0;

ESP32analogReadNonBlocking ADC_pin32(32, 100000);     // Solar Panel
ESP32analogReadNonBlocking ADC_pin33(33, 100000);
uint8_t ArbitrationToken1; //Use one Aribtration Token for anything on ADC1, GPIO: 34, 35, 36, 37, 38, 39ADC_
uint8_t ArbitrationToken2; //Use one Arbitration Token for anything on ADC2, GPIO: 4, 12, 13, 14, 15, 25, 26, 27   // do not use when WIFI is enabled

uint32_t loopcounter;//loop counter for example to show that the code is non-blocking
uint32_t loopcounterSerialPrintTimer;


//--------------------------------------------------------------------------
// Servo Setup
//--------------------------------------------------------------------------
#include <ESP32Servo.h>
Servo myservo;  // create servo object to control a servo
int pos = 0;    // variable to store the servo position
// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33
int servoPin = 18;


//--------------------------------------------------------------------------
// U8G2 Display Setup
//--------------------------------------------------------------------------
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // ESP32 Thing, HW I2C with pin remapping
#define U8LOG_WIDTH 64
#define U8LOG_HEIGHT 4
#define SUN	0
#define SUN_CLOUD  1
#define CLOUD 2
#define RAIN 3
#define THUNDER 4

// allocate memory
uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];
// Create a U8g2log object
U8G2LOG u8g2log;

//--------------------------------------------------------------------------
// ADC Setup
//--------------------------------------------------------------------------

int mVperAmp = 185;
int RawValue = 0;
int ACSoffset = 2.5;
double Voltage = 0;
double Amps = 0;


void log_display( String s)
  {
    u8g2log.print(s);
    u8g2log.print("\n");
    Serial.println(s);
  }


void setup_wifi() {
  // We start by connecting to a WiFi network  
  log_display("Connecting to ");
  log_display(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    log_display(".");
  }

  log_display("");
  log_display("WiFi connected");
  log_display("IP address: ");
  log_display(String(WiFi.localIP()));
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");

  u8g2log.print(topic); u8g2log.print("\n");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    u8g2log.print((char)payload[i]);
  }
  Serial.println();
  u8g2log.print("\n");

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == 's') {
    find_the_sun();

  } else {

  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(topic, "connected");
      // ... and resubscribe
      client.subscribe("mrflexi/solarserver/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



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



int get_max_insolation_angle( void )
{

  int max_value = 0;
  int max_angle = 0;

  myservo.write(0);
  delay(100);

  for ( int d = 0; d <= 180; d++)
  {

    myservo.write(d);
    delay(5);

    RawValue = analogRead(ADC25);


    if ( RawValue > max_value)
    {
      max_value = RawValue;
      max_angle = d;
    }

    u8g2log.print("Pos:"); u8g2log.print(d); u8g2log.print("\n");
    u8g2log.print("ADC25:"); u8g2log.print(RawValue); u8g2log.print("\n");
    u8g2log.print("Best Pos:"); u8g2log.print(max_angle); u8g2log.print("\n");
    u8g2log.print("Best ADC25:"); u8g2log.print(max_value); u8g2log.print("\n");

    mqtt_send_position(RawValue, d);
  }
  return max_angle;
}

void Calcula_corrente()
{
  //RawValue = analogRead(ADC1_CHANNEL_6);
  Voltage = RawValue * ( 3.3 / 4095.0);

  Amps = ((Voltage - ACSoffset) / mVperAmp);


  Serial.printf("%4d\ %4.2f\ %4.2f\n", RawValue, Voltage, Amps);

}


void mqtt_send_position(int voltage, int angle)
{
    
  doc["sensor"]            = "panel";
  doc["angle"]             = String(angle);
  doc["value"]             = String(voltage);

  JsonArray data = doc.createNestedArray("data");
  data.add(48.756080);
  data.add(2.302038);

  serializeJson(doc, msg);

  client.publish("mrflexi/solarserver/all", msg );
  Serial.print("Publish message: ");
  Serial.println(msg);

}


double GetVoltage(double reading) {
  // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
  if (reading < 1 || reading > 4095) return 0;
  // return -0.000000000009824 * pow(reading,3) + 0.000000016557283 * pow(reading,2) + 0.000854596860691 * reading + 0.065440348345433;
  return (-0.000000000000016 * pow(reading, 4) + 0.000000000118171 * pow(reading, 3) - 0.000000301211691 * pow(reading, 2) + 0.001109019271794 * reading + 0.034143524634089) + 0.15;
}



void find_the_sun( void )
{
  int best_angle = 0;

  best_angle = get_max_insolation_angle( );
  myservo.write(best_angle);
}

void setup()
{
  Serial.begin(115200);
  setup_display();

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(servoPin, 1000, 2000);
  delay(1000); // give me times to bring up serial monitor

  u8g2log.print("SolarServer"); u8g2log.print("\n");
  u8g2log.print("Seeking best pos"); u8g2log.print("\n");


}

void loop()
{


  // MQTT Connection
  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  // ADC Ports
  ADC_pin32.tick(ArbitrationToken1);
  ADC_pin33.tick(ArbitrationToken1);



  if (ADC_pin32.newValueFlag) {
    Serial.print("GPIO25 raw counts = ");
    Serial.print(ADC_pin32.counts);
    Voltage = ADC_pin32.counts * ( 3.3 / 4095.0);
    Serial.print(" Voltage: ");
    Serial.print(Voltage);
    Serial.print(" Voltage new: ");
    Voltage =  GetVoltage(ADC_pin32.counts);
    Serial.println(Voltage);
    u8g2log.print("Raw ADC0:"); u8g2log.print(ADC_pin32.counts); u8g2log.print("\n");
    u8g2log.print("Raw ADC1:"); u8g2log.print(ADC_pin33.counts); u8g2log.print("\n");
    u8g2log.print("Voltage:"); u8g2log.print(Voltage); u8g2log.print("\n");
    //Calcula_corrente();
  }








}
