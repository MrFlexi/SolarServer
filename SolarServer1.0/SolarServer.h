#ifndef SolarServer_h
#define SolarServer_h
#include "Arduino.h"
#include <U8g2lib.h>


#define U8LOG_WIDTH 64
#define U8LOG_HEIGHT 4
#define SUN	0
#define SUN_CLOUD  1
#define CLOUD 2
#define RAIN 3
#define THUNDER 4
#define SLEEP 10


//--------------------------------------------------------------------------
// ADC Setup
//--------------------------------------------------------------------------

int mVperAmp = 185;
int RawValue = 0;
int ACSoffset = 2.5;
double Voltage = 0;
double Amps = 0;



//--------------------------------------------------------------------------
// U8G2 Display Setup
//--------------------------------------------------------------------------
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // ESP32 Thing, HW I2C with pin remapping


void drawSymbol(u8g2_uint_t x, u8g2_uint_t y, uint8_t symbol)
{
  // fonts used:
  // u8g2_font_open_iconic_embedded_6x_t
  // u8g2_font_open_iconic_weather_6x_t
  // encoding values, see: https://github.com/olikraus/u8g2/wiki/fntgrpiconic
  
  switch(symbol)
  {
    case SUN:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(x, y, 69);	
      break;
    case SUN_CLOUD:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(x, y, 65);	
      break;
    case CLOUD:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(x, y, 64);	
      break;
    case RAIN:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(x, y, 67);	
      break;    
    case THUNDER:
      u8g2.setFont(u8g2_font_open_iconic_embedded_6x_t);
      u8g2.drawGlyph(x, y, 67);
      break;
   case SLEEP:
      u8g2.setFont( u8g2_font_open_iconic_all_8x_t);
      u8g2.drawGlyph(x, y, 67);  
      break;         
  }
}

void drawWeather(uint8_t symbol, int degree)
{
  drawSymbol(0, 48, symbol);
  u8g2.setFont(u8g2_font_logisoso32_tf);
  u8g2.setCursor(48+3, 42);
  u8g2.print(degree);
  u8g2.print("°C");		// requires enableUTF8Print()
}

void drawRawValue(uint8_t symbol, int degree, int voltage)
{
  Serial.print("drawRawValue");
  u8g2.firstPage();
  drawSymbol(0, 48, symbol);
  u8g2.setFont(u8g2_font_logisoso16_tf);
  u8g2.setCursor(48+3, 20);
  u8g2.print(degree);u8g2.print(" °");
  u8g2.print("");	
  
  u8g2.setCursor(48+3, 42);
  u8g2.print(voltage);u8g2.print(" V");
  u8g2.print("");	

  while ( u8g2.nextPage() );
  delay(10);
}

void drawLog(String s1, String s2, String s3, String s4, String s5)
{
  u8g2.clearBuffer();
  u8g2.firstPage();  
  u8g2.setFont(u8g2_font_8x13_mf);
  u8g2.setCursor(1, 10);
  u8g2.print(s1);
  u8g2.print("");  
  u8g2.setCursor(1, 26);  
  u8g2.print(s2);
  u8g2.print("");
  u8g2.setCursor(1, 40);  
  u8g2.print(s3);
  u8g2.print("");
  u8g2.setCursor(1, 54);  
  u8g2.print(s4);
  u8g2.print("");
  u8g2.setCursor(1, 57);
  u8g2.print(s5);
  u8g2.print("");
  while ( u8g2.nextPage() );  
}





/*
  Draw a string with specified pixel offset. 
  The offset can be negative.
  Limitation: The monochrome font with 8 pixel per glyph
*/
void drawScrollString(int16_t offset, const char *s)
{
  static char buf[36];	// should for screen with up to 256 pixel width 
  size_t len;
  size_t char_offset = 0;
  u8g2_uint_t dx = 0;
  size_t visible = 0;
  len = strlen(s);
  if ( offset < 0 )
  {
    char_offset = (-offset)/8;
    dx = offset + char_offset*8;
    if ( char_offset >= u8g2.getDisplayWidth()/8 )
      return;
    visible = u8g2.getDisplayWidth()/8-char_offset+1;
    strncpy(buf, s, visible);
    buf[visible] = '\0';
    u8g2.setFont(u8g2_font_8x13_mf);
    u8g2.drawStr(char_offset*8-dx, 62, buf);
  }
  else
  {
    char_offset = offset / 8;
    if ( char_offset >= len )
      return;	// nothing visible
    dx = offset - char_offset*8;
    visible = len - char_offset;
    if ( visible > u8g2.getDisplayWidth()/8+1 )
      visible = u8g2.getDisplayWidth()/8+1;
    strncpy(buf, s+char_offset, visible);
    buf[visible] = '\0';
    u8g2.setFont(u8g2_font_8x13_mf);
    u8g2.drawStr(-dx, 62, buf);
  }
  
}

void draw(const char *s, uint8_t symbol, int degree)
{
  int16_t offset = -(int16_t)u8g2.getDisplayWidth();
  int16_t len = strlen(s);
  for(;;)
  {
    u8g2.firstPage();
    do {
      drawWeather(symbol, degree);
      drawScrollString(offset, s);
    } while ( u8g2.nextPage() );
    delay(5);
    offset+=2;
    if ( offset > len*8+1 )
      break;
  }
}


double GetVoltage(double reading) {
  // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
  if (reading < 1 || reading > 4095) return 0;
  // return -0.000000000009824 * pow(reading,3) + 0.000000016557283 * pow(reading,2) + 0.000854596860691 * reading + 0.065440348345433;
  return (-0.000000000000016 * pow(reading, 4) + 0.000000000118171 * pow(reading, 3) - 0.000000301211691 * pow(reading, 2) + 0.001109019271794 * reading + 0.034143524634089) + 0.15;
}

void Calcula_corrente()
{
  //RawValue = analogRead(ADC1_CHANNEL_6);
  Voltage = RawValue * ( 3.3 / 4095.0);

  Amps = ((Voltage - ACSoffset) / mVperAmp);

  Serial.printf("%4d\ %4.2f\ %4.2f\n", RawValue, Voltage, Amps);

}


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


#endif
