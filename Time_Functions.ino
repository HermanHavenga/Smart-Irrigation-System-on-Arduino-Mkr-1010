///////////////////////////////////////////////
//get Epoch to set Real Time Clock
///////////////////////////////////////////////
void GetEpoch()
{
  unsigned long epoch;
  int numberOfTries = 0, maxTries = 6;
  do {
    epoch = WiFi.getTime();
    numberOfTries++;
  }
  while ((epoch == 0) && (numberOfTries < maxTries));

  if (numberOfTries == maxTries) {
    Serial.println("NTP unreachable!!");
    retryTime = 1;
  }
  else {
    retryTime = 0;
    Serial.print("Epoch received: ");
    Serial.println(epoch);
    rtc.setEpoch(epoch);
  }
}

///////////////////////////////////////////////
//get Time in minutes
///////////////////////////////////////////////
int timeInMin()
{
  int h = rtc.getHours();
  int m = rtc.getMinutes();
  int Min = (h*60)+m;
  return Min;
}

void setAlarm(int seconds)
{
  int sNow = rtc.getSeconds();
  seconds=sNow+seconds;
  if(seconds>=60)
  {
    seconds=seconds-60;
  }
    
  rtc.setAlarmTime(0, 0, seconds);
  rtc.enableAlarm(rtc.MATCH_SS);
  
  rtc.attachInterrupt(alarmMatch);
}

void alarmMatch()
{
  alarm = true;
}

///////////////////////////////////////////////
//get Day of the Week in binary
///////////////////////////////////////////////
int GetDoW()
{
  int c = 20;          // Century (ie 2016 would be 20)
  int mTable;     // Month value based on calculation table
  int SummedDate; // Add values combined in prep for Mod7 calc
  int DoW;        // Day of the week value (0-6)
  int leap;       // Leap Year or not
  int cTable = 0;     // Century value based on calculation table
  int d = rtc.getDay();
  int m = rtc.getMonth();
  int yy = rtc.getYear();
  int yyyy = 2000 + rtc.getYear();

  
    
  // Leap Year Calculation
  if((fmod(yyyy,4) == 0 && fmod(yyyy,100) != 0) || (fmod(yyyy,400) == 0))
  { leap = 1; }
  else 
  { leap = 0; }

  // Jan and Feb calculations affected by leap years
  if(m == 1)
  { if(leap == 1) { mTable = 6; }
    else          { mTable = 0; }}
  if(m == 2)
  { if(leap == 1) { mTable = 2; }
    else          { mTable = 3; }}
  // Other months not affected and have set values
  if(m == 10) { mTable = 0; }
  if(m == 8) { mTable = 2; }
  if(m == 3 || m == 11) { mTable = 3; }
  if(m == 4 || m == 7) { mTable = 6; }
  if(m == 5) { mTable = 1; }
  if(m == 6) { mTable = 4; }
  if(m == 9 || m == 12) { mTable = 5; }

  // Enter the data into the formula
  SummedDate = d + mTable + yy + (yy/4) + cTable - 1;
  
  // Find remainder
  DoW = fmod(SummedDate,7);  

  // Conver Day of Week to binary string
  if(DoW == 0) { DoW = 64;}                    //Sunday    - Binary = 0100 0000
  if(DoW == 1) { DoW = 32;}                    //Monday    - Binary = 0010 0000
  if(DoW == 2) { DoW = 16;}                    //Tuesday   - Binary = 0001 0000
  if(DoW == 3) { DoW = 8; }                    //Wednesday - Binary = 0000 1000
  if(DoW == 4) { DoW = 4; }                    //Thursday  - Binary = 0000 0100
  if(DoW == 5) { DoW = 2; }                    //Friday    - Binary = 0000 0010
  if(DoW == 6) { DoW = 1; }                    //Saturday  - Binary = 0000 0001

  
  Serial.println("DOW: " + String(DoW));
  return DoW;
}
