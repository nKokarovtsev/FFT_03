#include "arduinoFFT.h"
#include <SoftwareSerial.h>

arduinoFFT FFT;
/*
These values can be changed in order to evaluate the functions
*/
#define CHANNEL A3
const uint16_t samples = 128; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 6000; //Hz, must be less than 10000 due to ADC

unsigned int sampling_period_us;
unsigned long microseconds;

/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double vReal[samples];
double vImag[samples];
String payload = "";
int check = 0;
int flag = 0;

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

SoftwareSerial wemos(3,4);

void setup()
{
  /*pinMode(3, INPUT);
  pinMode(4, OUTPUT);

  digitalWrite(3, 1);
  digitalWrite(4, 0);*/

  sampling_period_us = round(1000000*(1.0/samplingFrequency));
  Serial.begin(115200);
  wemos.begin(115200);
  while(!Serial);
  Serial.println("Ready");
}

void loop()
{
  flag = 0;
  if(wemos.parseInt()==1984){
    /*SAMPLING*/
    delay(100);
    if(wemos.parseInt()!=3)
      coc(); //hz kak rabotaet, no hrenovo
    savve:
    delay(10000);
  }
}
void coc()
{
  microseconds = micros();
    for(int i=0; i<samples; i++)
    {
        vReal[i] = analogRead(CHANNEL);
        vImag[i] = 0;
        while(micros() - microseconds < sampling_period_us){
          //empty loop
        }
        microseconds += sampling_period_us;
    }
    FFT = arduinoFFT(vReal, vImag, samples, samplingFrequency); /* Create FFT object */
    /* Print the results of the sampling according to time */
    Serial.println("Data:");
    //PrintVector(vReal, samples, SCL_TIME);
    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);	/* Weigh data */
    Serial.println("Weighed data:");
    //PrintVector(vReal, samples, SCL_TIME);
    FFT.Compute(FFT_FORWARD); /* Compute FFT */
    Serial.println("Computed Real values:");
    //PrintVector(vReal, samples, SCL_INDEX);
    Serial.println("Computed Imaginary values:");
    //PrintVector(vImag, samples, SCL_INDEX);
    FFT.ComplexToMagnitude(); /* Compute magnitudes */
    Serial.println("Computed magnitudes:");
    PrintVector(vReal, (samples >> 1), SCL_FREQUENCY);
    double x = FFT.MajorPeak();
    Serial.println(x, 6); //Print out what frequency is the most dominant.
    wemos.flush();
    while (!wemos.parseInt()==99) {
      wemos.flush();
      delay(50);
      Serial.println(99);
      flag++;
      if (flag>200) {goto sas;}
    }
    wemos.println(x, 6);
    sas: ;
    //while(1); /* Run Once */
 /* Repeat after delay */
}

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  wemos.print("!");
  check = 0;
  while (!wemos.available()){
    flag++;
    delay(25);
    if (flag>50) {goto savve;}
  }
  check = wemos.parseInt();
  while (check != 228 && check!=993)
  {
    wemos.flush();
    delay(100);
    check = wemos.parseInt();
    Serial.println(String(check));
    Serial.println(228);
    flag++;
    if (flag>200) {goto savve;}
  }
  //wemos.println("!");
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    check = 0;
    while (!wemos.available()){}
    check = wemos.parseInt();
    while (check!=993) {
      wemos.flush();
      delay(100);
      check = wemos.parseInt();
      Serial.println(993);
      Serial.println(check);
      flag++;
    if (flag>200) {goto savve;}
    }
    payload = "";
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
	break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / samplingFrequency);
	break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * samplingFrequency) / samples);
	break;
    }
    //wemos.print(abscissa, 6);
    //if (wemos.parseInt()==228){
    //wemos.flush();
    payload = String(abscissa, 3) + " : " + String(vData[i], 4) + "\n";
    Serial.print(payload);
    payload.substring(0,-1);
    wemos.print(payload);
    delay(100);
    wemos.flush();
    //Serial.flush();
      //wemos.print(" ");
      //wemos.println(vData[i], 4);
    //}
    //if(scaleType==SCL_FREQUENCY)
      //wemos.print("Hz");
  }
  delay(4000);
  wemos.print("!");
  savve:
  delay(4000);
  //wemos.println();
}
