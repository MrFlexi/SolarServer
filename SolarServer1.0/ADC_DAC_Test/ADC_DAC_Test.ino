#define LED 2 

uint8_t DAC_Channel_1 = 25; 
uint8_t ADC19 = 26; 
uint8_t dac; 
uint16_t adc; 
int16_t diff = 0; 
double dcout = 0;
double adcvolt = 0;

void setup() 
{   
  Serial.begin(115200);   
  pinMode(LED, OUTPUT);   
  delay(1500);   


  dac = 64;
  dacWrite(DAC_Channel_1, dac);
  dcout = (3.3/255)*dac;
  delay(200); 
  adc = analogRead(ADC19);  

  adcvolt = (3.3/4095) * adc;
  diff = 16 * dac;     
  Serial.printf("%4d\t%6d\t%4d\t%.2f\t%.2f\n", dac, adc, diff, dcout ,adcvolt ); 
  delay(10000);

  dac = 128;
  dacWrite(DAC_Channel_1, dac);
  dcout = (3.3/255)*dac;
  delay(200); 
  adc = analogRead(ADC19);  

  adcvolt = (3.3/4095) * adc;
  diff = 16 * dac;     
  Serial.printf("%4d\t%6d\t%4d\t%.2f\t%.2f\n", dac, adc, diff, dcout ,adcvolt ); 
 
  delay(10000);
  Serial.println("-----------------------------------------------------------------");
  
  
} 

void loop() 
{     
  for (dac = 0; dac < 256; dac = dac + 8)   
  {     
  
  dacWrite(DAC_Channel_1, dac);
  dcout = (3.3/256)*dac;
  delay(200); 
  adc = analogRead(ADC19);  

  adcvolt = (3.3/4096) * adc;
  diff = 16 * dac;     
  Serial.printf("%.2f\t%.2f\n", dcout ,adcvolt ); 


    
  } 
}
