void connectWiFi (void)
{
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED)
  {
    attempt++;
    Serial.println(String(wifiNum) + " " + String(attempt));
    if(wifiNum == 1)
    {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid1);
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(wifiOnPin, LOW);
      status = WiFi.begin(ssid1, pass1);
    }
    if(wifiNum == 2)
    {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid2);
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(wifiOnPin, LOW);
      status = WiFi.begin(ssid2, pass2);
    }
    if(attempt > 1)
    {
      if(wifiNum ==1) {wifiNum=2;}
      else {wifiNum=1;}
    }

    if(testWiFi())
    {
      Serial.println("Connected to wifi");
      ip = WiFi.localIP();
      Serial.println(ip);
    }
  }
}

boolean testWiFi (void)
{
  boolean wifiOn = false;
  int pingResult = WiFi.ping(hostName);

  if (pingResult >= 0) 
  {
//    Serial.print("SUCCESS! RTT = ");
//    Serial.print(pingResult);
//    Serial.println(" ms");
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(wifiOnPin, HIGH);
    wifiOn =true;
  }
  else 
  {
    Serial.print("FAILED! Error code: ");
    Serial.println(pingResult);
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(wifiOnPin, LOW);
  }
  return wifiOn;
}
