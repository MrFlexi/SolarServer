#define LED 2

uint8_t DAC_Channel_1 = 25;
uint8_t ADC19 = 26;
uint8_t dac;
uint16_t adc;
int16_t diff = 0;
double dcout = 0;
double adcvolt = 0;
double adc_poly = 0;


double ReadVoltage(byte pin) {
  double reading = analogRead(pin); // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
  if (reading < 1 || reading > 4095) return 0;
  return -0.000000000000016 * pow(reading, 4) + 0.000000000118171 * pow(reading, 3) - 0.000000301211691 * pow(reading, 2) + 0.001109019271794 * reading + 0.034143524634089;
}


void setup()
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  delay(1500);


  dac = 64;
  dacWrite(DAC_Channel_1, dac);
  dcout = (3.3 / 255) * dac;
  delay(200);
  adc = analogRead(ADC19);

  adcvolt = (3.3 / 4095) * adc;
  diff = 16 * dac;
  Serial.printf("%4d\t%6d\t%4d\t%.2f\t%.2f\n", dac, adc, diff, dcout , adcvolt );
  delay(10000);

  dac = 128;
  dacWrite(DAC_Channel_1, dac);
  dcout = (3.3 / 255) * dac;
  delay(200);
  adc = analogRead(ADC19);

  adcvolt = (3.3 / 4095) * adc;
  diff = 16 * dac;
  Serial.printf("%6d\t%8d\t%4d\t%.2f\t%.2f\n", dac, adc, diff, dcout , adcvolt );

  delay(10000);
  Serial.println("-----------------------------------------------------------------");


}

void loop()
{

  for (dac = 0; dac < 257; dac = dac + 2)
  {

    dacWrite(DAC_Channel_1, dac);
    dcout = (3.3 / 255) * dac;
    delay(200);
    adc = analogRead(ADC19);
    adc_poly = ReadVoltage(ADC19),

    adcvolt = (3.3 / 4095) * adc;
    diff = 16 * dac;

    Serial.printf("%6d\t%8d\t%4d\t%.2f\t%.2f\t%.2f\n", dac, adc, diff, dcout , adcvolt, adc_poly  );

  }
}
