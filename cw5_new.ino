#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>
#include <Adafruit_SSD1331.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <SPI.h>
#include <gfxfont.h>
// pins for DC Motor
#define ENABLE 7
#define DIRA 5
#define DIRB 6
// pins for RGB LED
#define R 2
#define G 3
#define B 4
// pins for OLED Display
#define sclk 13
#define mosi 11
#define cs   10
#define rst  9
#define dc   8
// pin of the active buzzer
#define BUZZ 12
// color code for OLED Display
#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF
// pin for Temperature sensor
#define TEMP A0
// pin for Water level sensor
#define LEVEL A1
// pin for Tilt ball switch
#define TILT A2

int tempReading = 0;
int water_level = 0;
int tiltVal = 0;
int tiltThreshold = 800;
int tempThreshold = 24;
int waterTarget = 3000;
int waterConsumed = 0;
int waterToDrink = 10;
float tempC = 0.0;
float tempF = 0.0;

bool ifTempC = true;
bool ifTempF = !ifTempC;



Adafruit_SSD1331 display = Adafruit_SSD1331(&SPI, cs, dc, rst);

//main thread
Thread mainThread = Thread();

// callback for mainThread
void mainCallback(){

  tiltVal = analogRead(TILT);
  if(tiltVal>tiltThreshold)
  {
    Serial.println("Mug tilted!");
    tiltDisplay();
    resetLED();

    while(tiltVal>tiltThreshold){
      tiltWarning();
      tiltVal = analogRead(TILT);
    }

    display.fillScreen(BLACK);
    Serial.println("Warning sent!");

  }else
  {
    Serial.println("Mug placed alright!");
    safeDisplay();
    Serial.println("Warning cancelled!");
    prepTempLevelDisplay();

    while(tiltVal<=tiltThreshold){

      tempC = readTempC();
      tempF = readTempF();
      
      int new_water_level = analogRead(LEVEL);
      if(new_water_level<water_level-10){
        waterConsumed += (water_level - new_water_level);
      }
      water_level = new_water_level;

      TempLevelDisplay();

      tempLED();

      Serial.println(tempC/100.0*255.0);
      Serial.println(int(tempC/100.0*255.0));


      while(tempC>=tempThreshold && tiltVal <= tiltThreshold){

        Serial.println("Temperature above threshold, Fan on!");
        fanOn();

        tempC = readTempC();
        tempF = readTempF();

        tiltVal = analogRead(TILT);

        if(tempC<tempThreshold || tiltVal > tiltThreshold){
          fanOff();
          break;
        }

        // perform water level calculation
        TempLevelDisplay();

      }

      fanOff();

      delay(1000);
      tiltVal = analogRead(TILT);
    }

  }
}

void resetLED(){
  digitalWrite(R, LOW);
  digitalWrite(G, LOW);
  digitalWrite(B, LOW);
}

void tempLED(){
  analogWrite(G, 0);
  analogWrite(B, int(255-tempC/100*255));
  analogWrite(R, int(255+tempC/100*255));
}

void fanOn(){
  digitalWrite(ENABLE,HIGH); // enable on
  digitalWrite(DIRA,HIGH); //one way
  digitalWrite(DIRB,LOW);
}

void fanOff(){
  digitalWrite(ENABLE,LOW); // disable
  digitalWrite(DIRA,LOW); //one way
  digitalWrite(DIRB,LOW);
}

float readTempC(){
  // perform temperature check
  tempReading = analogRead(TEMP);

  double tempK = log(10000.0 * ((1024.0 / tempReading - 1)));
  tempK = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK )) * tempK );       //  Temp in Kelvin
  float tempC = tempK - 273.15;            // Convert Kelvin to Celcius
  float tempF = (tempC * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit

  return tempC;
}

float readTempF(){
  // perform temperature check
  tempReading = analogRead(TEMP);

  double tempK = log(10000.0 * ((1024.0 / tempReading - 1)));
  tempK = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK )) * tempK );       //  Temp in Kelvin
  float tempC = tempK - 273.15;            // Convert Kelvin to Celcius
  float tempF = (tempC * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit

  return tempF;
}
void tiltDisplay(){
  display.fillScreen(BLACK);
  display.setCursor(0, 5);
  display.setTextColor(RED);
  display.setTextSize(2);
  display.println("HEEEELP!");
}

void safeDisplay(){
  display.fillScreen(BLACK);
  display.setCursor(0, 5);
  display.setTextColor(GREEN);
  display.setTextSize(2);
  display.println("SAFE NOW");
  delay(2000);
}

void prepTempLevelDisplay(){
  display.fillScreen(BLACK);
  display.setCursor(0, 5);
  display.setTextColor(GREEN);
  display.setTextSize(1);
  display.println("TEMP  ");
  display.println("      ");
  display.println("LEVEL ");
  display.println("      ");
//  display.println("Happy drinking water!");
}

void TempLevelDisplay(){
  display.fillRect((display.width()-23)/2, int16_t(0) , (display.width()+23)/2, (display.height()+7)/2, BLACK);

  display.setCursor(0, 5);
  display.setTextColor(GREEN);
  display.setTextSize(2);
  display.print("   ");

  if(ifTempC){
    display.print(int(tempC));
    display.setTextSize(1);
    display.print(" C");
  }else{
    display.print(int(tempF));
    display.setTextSize(1);
    display.print(" F");
  }

  display.setTextSize(2);
  display.println(" ");
  display.print("   ");
  display.print(water_level);
  display.setTextSize(1);
  display.println(" ml");

  waterToDrink = waterTarget-waterConsumed;
  if (waterToDrink>0){
      display.println(" ");
      display.println(" ");
      display.print("TODO ");
      display.setTextSize(2);
    //  display.println(" ");
    //  display.print("   ");
    display.fillRect((display.width()-36)/2, (display.height()+10)/2 , display.width()-20, display.height(), BLACK);
      display.print(waterToDrink);
      
      display.setTextSize(1);
      display.println(" ml");
  } else {
      display.fillScreen(BLACK);
      display.setCursor(0, 5);
      display.setTextColor(BLUE);
      display.setTextSize(3);
      display.println("NICE!");
      display.setTextSize(1);
      display.println("You have finished the water drinking goal today!");
  }

}

void tiltWarning(){
  unsigned int i;
  //output an frequency
  for(i=0;i<80;i++)
  {
    digitalWrite(BUZZ,HIGH);
    delay(1);//wait for 1ms
    digitalWrite(BUZZ,LOW);
    delay(1);//wait for 1ms
  }

  digitalWrite(R,HIGH);

  //output another frequency
  for(i=0;i<100;i++)
  {
    digitalWrite(BUZZ,HIGH);
    delay(2);//wait for 2ms
    digitalWrite(BUZZ,LOW);
    delay(2);//wait for 2ms
  }

  digitalWrite(R,LOW);

}

void start_screen() {
  display.fillScreen(BLACK);
  display.setCursor(0, 5);
  display.setTextColor(GREEN);
  display.setTextSize(2);
  display.println("SMART");
  display.println("MUG");
}


void setup(){
  Serial.begin(9600);

  display.begin();
  Serial.println("OLED Display initiated!");
  start_screen();

  // Set the tilt pin to be high
  digitalWrite(TILT, HIGH);

  // initialize DC Motor pins as outputs
  pinMode(ENABLE,OUTPUT);
  pinMode(DIRA,OUTPUT);
  pinMode(DIRB,OUTPUT);
  Serial.println("DC Motor pins initiated!");

  // initialize the buzzer pin as an output
  pinMode(BUZZ,OUTPUT);
  Serial.println("Buzzer pins initiated!");

  // initialize RGB LED pins as outputs
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  // Starting the mug LED shows green
  digitalWrite(R, LOW);
  digitalWrite(G, HIGH);
  digitalWrite(B, LOW);
  Serial.println("RGB LED pins initiated!");


  // Configure myThread
  mainThread.onRun(mainCallback);
  mainThread.setInterval(500);


  delay(2000);

  display.fillScreen(BLACK);

}

void loop(){

  mainThread.run();

}
