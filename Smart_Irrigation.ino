#include <ArduinoJson.h>
#include <WiFiSSLClient.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <RTCZero.h>
#include <Adafruit_SleepyDog.h>
#include "ThingSpeak.h"
#include "Secret.h"

//WIFI
  IPAddress ip;
  char ssid1[] = SECRET_SSID1; //  your network SSID1 (name)
  char pass1[] = SECRET_PSW1;//  your network PASSWORD1 ()
  char ssid2[] = SECRET_SSID2; //  your network SSID2 (name)
  char pass2[] = SECRET_PSW2;//  your network PASSWORD2 ()
  int wifiNum = 1;
  int attempt = 0;
  String hostName = "www.google.com";
  int wifiOnPin = 3;
  int powerPin = 2;

  WiFiClient clientWheather;
  WiFiClient clientTS;
  WiFiClient clientTS2;
  
 //WiFi status
  int status = WL_IDLE_STATUS; 

 //ThingSpeak Data
  unsigned long myChannelNumber = SECRET_CH_ID;
  const char * myWriteAPIKey = SECRET_WRITE_API;
  const char * myReadAPIKey = SECRET_READ_API;
  char serverTS[] = "api.thingspeak.com";

//MOISTURE
  int soilMoisture;
  int moisturePin = A1;

//TEMPERATURE
  // Data wire is conntec to the Arduino digital pin 0
  #define ONE_WIRE_BUS 0
  // Setup a oneWire instance to communicate with any OneWire devices
  OneWire oneWire(ONE_WIRE_BUS);
  // Pass our oneWire reference to Dallas Temperature sensor 
  DallasTemperature sensors(&oneWire);
  float tempValue;

//HUMIDITY
  // Data wire is conntec to the Arduino digital pin 2
  #define DHTPIN 1
  #define DHTTYPE DHT11
  DHT dht(DHTPIN, DHTTYPE);
  float HumData[] ={0,0,0};

//WEATHER
  String Weather[] ={" "," "," "};
  
  //open weather map api key
  String apiKey= SECRET_APIKEY;
  char serverW[] = "api.openweathermap.org";
  
  //the city you want the weather for
  String location= "1105777"; //Centurion

//RELAYS
  int relay_zone1=A2; //Output 1
  int relay_zone2=A3; //Output 2
  int Relay_State = 0;
  int Relay1_State = 0;
  int Relay2_State = 0;
  int autoStartTime1 = 0;
  int autoStartTime2 = 0;
  int relay1OnPin = 4;
  int relay2OnPin = 5;


//TIME
  RTCZero rtc;

  int retryTime=0;
  const int GMT = 2;
  int DoW = -1;
  int loopDelay = 1;
  int lastLoop = 0;
  bool alarm = false;
  int currentTime = 0;

//CONFIG
  int settings[11];

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  
  // Initialise built-in LED
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(wifiOnPin, OUTPUT);
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, HIGH);
    
  //Start up the Temp sensor
  sensors.begin();

  //Start up the Humidity sensor
  dht.begin(); 

  //Setup relays
  pinMode(relay_zone1,OUTPUT);
  pinMode(relay_zone2,OUTPUT);
  pinMode(relay1OnPin,OUTPUT);
  pinMode(relay2OnPin,OUTPUT);
  digitalWrite(relay_zone1, HIGH);
  digitalWrite(relay_zone2, HIGH);
  digitalWrite(relay1OnPin, LOW);
  digitalWrite(relay2OnPin, LOW);

  //Connect to Wifi network
  connectWiFi();

  //Initialize ThingSpeak
  ThingSpeak.begin(clientTS);  

  //Get online Time & Date
  rtc.begin();
  GetEpoch();

  //WatchDog
  Watchdog.enable(45000);

}

void loop() {

  Watchdog.reset();
  
  //Get online Time & Date
  if(retryTime == 1)
  { GetEpoch(); }
  
  //Get config on ThingSpeak
  getConfig();

  //Test WiFi connection
  if(!testWiFi())
  {
    connectWiFi();
  }
  
  currentTime = timeInMin();
  
  Serial.print(".");

//Only run functions every time the loop delay is over.
  if(lastLoop + loopDelay <= currentTime)
  {
    Serial.println(".");
    for(int i=0; i<11; i++)
    {
      Serial.print(String(i) + ": " + String(settings[i]) + " ");
    }
    Serial.println();
    lastLoop = currentTime;
  
    getWeather();
  
    DoW = GetDoW();   
    
    ///////////////////////////////////////////////
    //get all the data from the soil moisture sensor
    ///////////////////////////////////////////////
    soilMoisture = getAveMoisture();
  //  Serial.print("Current Moisture: ");
  //  Serial.println(soilMoisture);
  
    ///////////////////////////////////////////////
    //get all the data from the temperature sensor
    ///////////////////////////////////////////////
    tempValue = getTemperature();
  //  Serial.print("Actual temperature: ");
  //  Serial.println(tempValue);
  
    ///////////////////////////////////////////////
    //get all the data from the temperature sensor
    ///////////////////////////////////////////////
    getHumidity();
//    Serial.print("Actual humidity: ");
//    Serial.println(HumData[0]);
//    Serial.print("Actual heat index: ");
//    Serial.println(HumData[2]);
  
    // Print values.
  //  Serial.println("Online Weather: "+ Weather[0]);
  //  Serial.println("Online Temp: "+ Weather[1]);
  //  Serial.println("Online Humidity: "+ Weather[2]);
    
    ///////////////////////////////////////////////
    //verify the conditions.
    ///////////////////////////////////////////////
    int zone1Start = (settings[0]*60)+settings[1]-(GMT*60);
    int zone2Start = (settings[4]*60)+settings[5]-(GMT*60);

    Serial.println("Zone1 Time: " + String(zone1Start)+" "+String(currentTime)+" "+String(zone1Start+settings[2])); 
    Serial.println("Zone2 Time: " + String(zone2Start)+" "+String(currentTime)+" "+String(zone2Start+settings[6]));       
    Serial.println("Relay1: " + String(Relay1_State)+ " Relay2: " + String(Relay2_State));

  //Test Zone 1
    
    //Manual mode
    if(settings[9] == 0)
    {
      if((DoW & settings[3]) && (zone1Start <= currentTime) && (currentTime <= (zone1Start+settings[2])) && (Relay1_State == 0))
      {
        checkWeather("Zone1: ",relay_zone1, 1, relay1OnPin);
      }
      if((Relay1_State == 1) && (currentTime > (zone1Start+settings[2])))
      {
        digitalWrite(relay_zone1, HIGH);
        digitalWrite(relay1OnPin, LOW);
        Relay1_State = 0;
      }
    }
     
    //Auto mode
    if(settings[9] == 1)
    {
      if(Relay1_State == 0)
      {
        checkWeather("Zone1: ",relay_zone1, 1, relay1OnPin);
        autoStartTime1 = currentTime;
      }
      if((Relay1_State == 1) && (currentTime > (autoStartTime1+settings[2])))
      {
        digitalWrite(relay_zone1, HIGH);
        digitalWrite(relay1OnPin, LOW);
        Relay1_State = 0;
      }
    }
    
  //Test Zone 2

    //Manual mode
    if(settings[10] == 0)
    {
      if((DoW & settings[7]) && (zone2Start <= currentTime) && (currentTime <= (zone2Start+settings[6])) && (Relay2_State == 0))
      {
        checkWeather("Zone2: ",relay_zone2, 2, relay2OnPin);
      }
      if((Relay2_State == 1) && (currentTime > (zone2Start+settings[6])))
      {
        digitalWrite(relay_zone2, HIGH);
        digitalWrite(relay2OnPin, LOW);
        Relay2_State = 0;
      }
    }
     
    //Auto mode
    if(settings[10] == 1)
    {
      if(Relay2_State == 0)
      {
        checkWeather("Zone2: ",relay_zone2, 2, relay1OnPin);
        autoStartTime2 = currentTime;        
      }
      if((Relay2_State == 1) && (currentTime > (autoStartTime2+settings[6])))
      {
        digitalWrite(relay_zone2, HIGH);
        digitalWrite(relay2OnPin, LOW);
        Relay2_State = 0;
      }
    }
       
    if((Relay1_State == 0) && (Relay2_State == 0))
    {
       Relay_State = 0;
    }
    else {Relay_State = 1;}
           
    Serial.println("Relay1: " + String(Relay1_State)+ " Relay2: " + String(Relay2_State));
  
    ///////////////////////////////////////////////
    //Publish data to ThinkSpeak
    ///////////////////////////////////////////////
    Serial.print("Publish data to ThingSpeak");
    ThingSpeak.setField(1, tempValue);          //Sensor Temp
    ThingSpeak.setField(2, Weather[1]);         //Online Temp
    ThingSpeak.setField(3, HumData[2]);         //Heat Index
    ThingSpeak.setField(4, Relay_State);        //Relay State    
    ThingSpeak.setField(5, HumData[0]);         //Sensor Humidity
    ThingSpeak.setField(6, Weather[2]);         //Online Humidity
    ThingSpeak.setField(7, getAveMoisture());   //Sensor Moisture 
    ThingSpeak.setStatus(Weather[0]);           //Channel Status
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);  
    if(x==200)
    {
    Serial.println("...Done!");
    }
    else
    {
      Serial.println("...FAILED! (Error:" + String(x) + ")");
    }
  }
}
