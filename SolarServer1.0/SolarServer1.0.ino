

/******************************************************
  Solar Server 1.0
  Analog Input:

  MJRoBot.org 6Sept17
*****************************************************/

#define mqtt_off      // activate MQTT integration with mqtt_on
#define ESP_SLEEP_OFF // activate low enery sleep with ESP_SLEEP_ON

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <ESP32analogReadNonBlocking.h>
#include <ESP32Servo.h>
#include <U8g2lib.h>
#include "SolarServer.h"
#include <NTPClient.h> // Internet Time Server
#include <WiFiUdp.h>

#ifdef mqtt_on
#include <PubSubClient.h>
#endif

//--------------------------------------------------------------------------
// ESP Sleep Mode
//--------------------------------------------------------------------------

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60       // sleep for 1 minute

//--------------------------------------------------------------------------
// get time from internet
//--------------------------------------------------------------------------
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 7200); // 7200 = + 2h

//--------------------------------------------------------------------------
// Wifi Setup + MQTT
//--------------------------------------------------------------------------

const char *ssid = "MrFlexi";
const char *password = "Linde-123";
const char *mqtt_server = "192.168.1.144"; // Laptop
//const char* mqtt_server = "test.mosquitto.org";   // Laptop

const char *mqtt_topic = "mrflexi/solarserver/";

//--------------------------------------------------------------------------
// JSON Setup
//--------------------------------------------------------------------------

DynamicJsonDocument doc(1024);
char msg[200];
String str = "";
String str1 = "";
String str2 = "";

WiFiClient espClient;

#ifdef mqtt_on
PubSubClient client(espClient);
long lastMsgAlive = 0;
long lastMsgDist = 0;
#endif

//--------------------------------------------------------------------------
// Analog Read Setup  ADC
//--------------------------------------------------------------------------

int ADC32 = 32;
int analog_value = 0;
int PanelPosition = 0;

ESP32analogReadNonBlocking ADC_pin32(32, 100000); // Solar Panel
ESP32analogReadNonBlocking ADC_pin33(33, 100000);
uint8_t ArbitrationToken1; //Use one Aribtration Token for anything on ADC1, GPIO: 34, 35, 36, 37, 38, 39ADC_
uint8_t ArbitrationToken2; //Use one Arbitration Token for anything on ADC2, GPIO: 4, 12, 13, 14, 15, 25, 26, 27   // do not use when WIFI is enabled

uint32_t loopcounter; //loop counter for example to show that the code is non-blocking
uint32_t loopcounterSerialPrintTimer;

//--------------------------------------------------------------------------
// Servo Setup
//--------------------------------------------------------------------------

Servo myservo; // create servo object to control a servo
int pos = 0;   // variable to store the servo position
// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33
int servoPin = 18;

// assume 4x6 font, define width and height
#define U8LOG_WIDTH 64
#define U8LOG_HEIGHT 4

// allocate memory
uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];
// Create a U8g2log object
U8G2LOG u8g2log;

void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
}

void log_display(String s)
{
  //u8g2log.print(s);
  //u8g2log.print("\n");
  Serial.println(s);
}

void setup_wifi()
{
  // We start by connecting to a WiFi network
  log_display("Connecting to ");
  log_display(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    log_display(".");
  }

  log_display("");
  log_display("WiFi connected");
  log_display("IP address: ");
  log_display(String(WiFi.localIP()));

  timeClient.begin();
}

#ifdef mqtt_on

void callback(char *mqtt_topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");

  u8g2log.print(mqtt_topic);
  u8g2log.print("\n");
  Serial.print(mqtt_topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    u8g2log.print((char)payload[i]);
  }
  Serial.println();
  u8g2log.print("\n");

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == 's')
  {
    find_the_sun();
  }
  else
  {
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client"))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(mqtt_topic, "connected");
      // ... and resubscribe
      client.subscribe("mrflexi/solarserver/#");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqtt_send_position(int voltage, int angle)
{

  doc["sensor"] = "panel";
  doc["angle"] = String(angle);
  doc["value"] = String(voltage);

  JsonArray data = doc.createNestedArray("data");
  data.add(48.756080);
  data.add(2.302038);

  serializeJson(doc, msg);

  client.publish("mrflexi/solarserver/all", msg);
  Serial.print("Publish message: ");
  Serial.println(msg);
}

#endif

void setup_display(void)
{
  u8g2.begin();
  u8g2log.begin(u8g2, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer); // connect to u8g2, assign buffer
  u8g2log.setLineHeightOffset(0);                               // set extra space between lines in pixel, this can be negative
  u8g2log.setRedrawMode(0);                                     // 0: Update screen with newline, 1: Update screen for every char
  u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
  u8g2.drawGlyph(1, 20, 69);
  u8g2.enableUTF8Print();
}

int get_max_insolation_angle(void)
{

  int max_value = 0;
  int max_angle = 0;

  for (int d = 0; d <= 180; d++)
  {

    myservo.write(d);
    delay(100);

    RawValue = analogRead(ADC32);
    if (RawValue > max_value)
    {
      max_value = RawValue;
      max_angle = d;
    }

    Serial.print("Pos: ");
    Serial.print(d);
    Serial.print("  Value: ");
    Serial.println(RawValue);

#ifdef mqtt_on
        mqtt_send_position(RawValue, d);
#endif
  }
  Serial.print("Best Pos: ");Serial.print(max_angle);Serial.print("  Value: ");Serial.println(max_value);

  return max_angle;
}

void find_the_sun(void)
{
  int best_angle = 0;
  myservo.setPeriodHertz(50); // standard 50 hz servo
  myservo.attach(servoPin, 1000, 2000);

  Serial.println("Seraching the sun");
  myservo.write(0);
  delay(1000);
  myservo.write(180);
  delay(1000);
  myservo.write(0);
  delay(1000);

  PanelPosition = get_max_insolation_angle();

  myservo.write(PanelPosition);
  delay(1000);
  myservo.detach();
  Serial.println("Best Position:" + PanelPosition);
}

void setup()
{
  Serial.begin(115200);
  delay(1000); // give me times to bring up serial monitor

  print_wakeup_reason();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
                 " Seconds");

  setup_display();
  setup_wifi();

#ifdef mqtt_on
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
#endif

  draw("Where is the sun", SUN, 27);
  find_the_sun();
}

int i = 0;

void loop()
{

#ifdef mqtt_on
  // MQTT Connection
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
#endif

  // ADC Ports
  ADC_pin32.tick(ArbitrationToken1);
  ADC_pin33.tick(ArbitrationToken1);

  if (ADC_pin32.newValueFlag)
  {
    Serial.print("GPIO32 raw counts = ");
    Serial.print(ADC_pin32.counts);
    Voltage = ADC_pin32.counts * (3.3 / 4095.0);
    Serial.print(" Voltage: ");
    Serial.print(Voltage);
    Serial.print(" Voltage new: ");
    Voltage = GetVoltage(ADC_pin32.counts);
    Serial.println(Voltage);
    
    if (ADC_pin32.counts < 2500 )
    {
      drawRawValue(RAIN, PanelPosition, ADC_pin32.counts );
    }
    else
    {
      drawRawValue(SUN, PanelPosition, ADC_pin32.counts );
    }
    
    
  }

  //timeClient.update();
  //Serial.println(timeClient.getFormattedTime());

  //draw( timeClient.getFormattedTime().c_str() , SUN, i);
  i++;
  //Serial.print("loop= "); Serial.println(i);
  //Serial.print("ADC32  = "); Serial.println( analogRead(ADC32));

#ifdef ESP_SLEEP_ON
  if (i > 5)
  {
    Serial.println("Going to sleep now");
    delay(1000);
    Serial.flush();
    esp_deep_sleep_start();
    Serial.println("This will never be printed");
  }
#endif
}
