///////////////////////////////////////////////
//Get average data from the soil moisture sensor
///////////////////////////////////////////////
int getAveMoisture()
{
  //Moisuture Scalling Variables
  int dryValue = 1023; //Lower limit of sensor
  int wetValue = 470;  //Upper limit of sensor
  int friendlyDryValue = 0; //Lower limit of range
  int friendlyWetValue = 100; //Upper limit of range
  int rawMoisture = 0;
  
  for (int a = 0; a < 10; a++) {
    rawMoisture += analogRead(moisturePin);
    delay(10);
  }
  rawMoisture = rawMoisture/10;
  
  int tempMoisture = map(rawMoisture, dryValue, wetValue, friendlyDryValue, friendlyWetValue);
  //Serial.println("Moisture: " + tempMoisture);
  
  return tempMoisture;
}

///////////////////////////////////////////////
//Get all the data from the temperature sensor
///////////////////////////////////////////////
float getTemperature() 
{
   sensors.requestTemperatures();
   return sensors.getTempCByIndex(0);
}

///////////////////////////////////////////////
//Get all the data from the humidity sensor
///////////////////////////////////////////////
float getHumidity() 
{
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  HumData[0] = h;
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  HumData[1] = t;
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);
  HumData[2] = hic;
}

///////////////////////////////////////////////
//Check if zones should be active
///////////////////////////////////////////////
void checkWeather(String zone,int relay, int state, int light)
{
  Serial.println(zone+"Checking weather conditions: ");
    if (soilMoisture<settings[8])
    {
      Serial.println(zone+"I feel a bit dry");
    
      if (Weather[0]=="rain"){
        Serial.println(zone+"I see clouds at the horizon!");
      }else{
        Serial.println(zone+"WATER!");
        digitalWrite(relay, LOW);
        digitalWrite(light, LOW);
        if(state == 1)
        {
          Relay1_State = 1;
          Serial.println(Relay1_State);
        }
       if(state == 2)
        {
          Relay2_State = 1;          
          Serial.println(Relay2_State);
        }
      }      
    }else{
      Serial.println(zone+"Off");
      digitalWrite(relay, HIGH);
      digitalWrite(light, HIGH);
      if(state == 1)
        {
          Relay1_State = 0;
          Serial.println(Relay1_State);
        }
      if(state == 2)
        {
          Relay2_State = 0;
          Serial.println(Relay2_State);
        }
    }
  }

///////////////////////////////////////////////
//get all the online data from the weather API
///////////////////////////////////////////////
void getWeather() 
{
  Serial.println("Gathering online weather data...");
  // if you get a connection, report back via serial:
  if (clientWheather.connect(serverW, 80)) {
    //Serial.println("connected to server");
    // Make a HTTP request:
    clientWheather.print("GET /data/2.5/weather?");
    clientWheather.print("id="+location);
    clientWheather.print("&appid="+apiKey);
    clientWheather.println("&units=metric");
    clientWheather.println("Host: api.openweathermap.org");
    clientWheather.println("Connection: close");
    clientWheather.println();
  } else {
    Serial.println("unable to connect");
  }

  delay(2500);
  String line = "";

  setAlarm(15);
  
  while (clientWheather.connected()&& alarm==false) 
  {
    line = clientWheather.readStringUntil('\n');
    //Serial.println(line);
    //create a json buffer where to store the json data
    StaticJsonBuffer<5000> jsonBuffer;

    JsonObject& root = jsonBuffer.parseObject(line);
    if (!root.success()) 
      {
      Serial.println("parseObject() failed");
      }
    else
    {
      //get the data from the json tree
      String currentWeather = root["weather"][0]["main"];
      String currentTemp = root["main"]["temp"];
      String currentHumidity = root["main"]["humidity"];
        
      Weather[0] = currentWeather;
      Weather[1] = currentTemp;
      Weather[2] = currentHumidity;
    }
  } 
  
  rtc.detachInterrupt();
  alarm = false;
}
