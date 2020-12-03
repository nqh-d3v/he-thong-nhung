/*

    Copyright (C) 2017 Libelium Comunicaciones Distribuidas S.L.
   http://www.libelium.com

    By using it you accept the MySignals Terms and Conditions.
    You can find them at: http://libelium.com/legal

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Version:           0.1
    Design:            David Gascon
    Implementation:    Luis Martin / Victor Boria
*/

#include <MySignals.h>
#include "Wire.h"
#include "SPI.h"


void setup()
{
  Serial.begin(115200);
  MySignals.begin();

  //Disable WiFi ESP8266 Power -> bit1:0
  bitClear(MySignals.expanderState, EXP_ESP8266_POWER);    
  MySignals.expanderWrite(MySignals.expanderState);

  delay(1000);
  
  //Enable WiFi ESP8266 Power -> bit1:1
  bitSet(MySignals.expanderState, EXP_ESP8266_POWER);
  MySignals.expanderWrite(MySignals.expanderState);

  MySignals.initSensorUART();

  MySignals.enableSensorUART(WIFI_ESP8266);
  delay(1000);


  // Checks if the WiFi module is started
  int8_t answer = sendATcommand("AT", "OK", 6000);
  if (answer == 0)
  {
    MySignals.println("Error");
    // waits for an answer from the module
    while (answer == 0)
    {
      // Send AT every two seconds and wait for the answer
      answer = sendATcommand("AT", "OK", 6000);
    }
  }
  else if (answer == 1)
  {
    MySignals.println("WiFi succesfully working!");
  }
  
  // Checks if the WiFi module is started
  answer = sendATcommand("AT+GSLP=10", "OK", 6000);
  if (answer == 0)
  {
    MySignals.println("AT+GSLP Error");
    
  }
  else if (answer == 1)
  {
    MySignals.println("AT+GSLP ok");
  }
  
}


void loop()
{

  uint16_t ecg = (uint16_t)MySignals.getECG(DATA);
  
  Serial.println(ecg);

  delay(5);

 
}


int8_t sendATcommand(char* ATcommand, char* expected_answer1, unsigned int timeout)
{

  uint8_t x = 0,  answer = 0;
  char response[50];
  unsigned long previous;

  memset(response, '\0', sizeof(response));    // Initialize the string

  delay(100);

  while ( Serial.available() > 0) Serial.read();   // Clean the input buffer

  delay(1000);
  Serial.println(ATcommand);    // Send the AT command

  x = 0;
  previous = millis();

  // this loop waits for the answer
  do
  {

    if (Serial.available() != 0)
    {
      response[x] = Serial.read();
      x++;
      // check if the desired answer is in the response of the module
      if (strstr(response, expected_answer1) != NULL)
      {
        answer = 1;
        //MySignals.println(response);

      }
    }
    // Waits for the asnwer with time out
  }
  while ((answer == 0) && ((millis() - previous) < timeout));

  return answer;
}

