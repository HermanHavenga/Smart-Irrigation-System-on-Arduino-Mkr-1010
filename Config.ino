///////////////////////////////////////////////
//Get the online config data from the ThingSpeak API
///////////////////////////////////////////////
void getConfig() 
{
  //Serial.println("Gathering config data...");
  // if you get a connection, report back via serial:
  if (clientTS2.connect(serverTS, 80)) {
    //Serial.println("connected to server");
    // Make a HTTP request:
    clientTS2.print("GET /channels/841372/feeds.xml?results=0&metadata=true");
    clientTS2.println(" HTTP/1.1");
    clientTS2.println("Accept-Encoding: gzip,deflate");
    clientTS2.println("Host: api.thingspeak.com");
    clientTS2.println("Connection: Close");
    clientTS2.println();
  } else {
    Serial.println("unable to connect");
  }

  delay(1000);
  String response="";
  setAlarm(5);
  
  while (clientTS2.connected()&& alarm==false) 
  {
    String temp = clientTS2.readStringUntil('\n');
    response.concat(temp);
  }
  //Serial.println(response);

  if(response.length() == 0)
  {
    Serial.println("No response from ThingSpeak");
  }
  else
  {
    rtc.detachInterrupt();
    alarm = false;
    
    if(response.indexOf("200 OK")>=0)
    {       
      int index1 = response.indexOf("<metadata>") + strlen("<metadata>");
      int index2 = response.indexOf("</metadata>"); 
      String metaData = response.substring(index1, index2);
      //Serial.println(metaData);
    
      int num[] ={0,2,3,5,6,8,9,12,13,15,16,18,19,21,22,25,26,28,29,30,31,32};
    
      for(int i=0; i < 11; i++)
      {
        String tempS = metaData.substring(num[i*2], num[(i*2)+1]);
        settings[i] = tempS.toInt();
      }  
    }
  }
}
    
