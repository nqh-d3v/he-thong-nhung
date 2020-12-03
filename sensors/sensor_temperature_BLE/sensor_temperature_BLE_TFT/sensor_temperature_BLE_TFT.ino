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

    Version:           2.0
    Design:            David Gascon
    Implementation:    Luis Martin / Victor Boria
*/

#include <Adafruit_GFX_AS.h>
#include <Adafruit_ILI9341_AS.h>
#include <MySignals.h>
#include <MySignals_BLE.h>

// Write here the MAC address of BLE device to find
char MAC_TEMPERATURE[14] = "20C38FD83131";

uint8_t available_temperature = 0;
uint8_t connected_temperature = 0;

float temperature;

#define TEMPERATURE_MEASURE_HANDLE 43

char buffer_tft[20];

Adafruit_ILI9341_AS tft = Adafruit_ILI9341_AS(TFT_CS, TFT_DC);

void setup()
{

  MySignals.begin();

  tft.init();
  tft.setRotation(2);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);

  //TFT message: Welcome to MySignals
  strcpy_P((char*)buffer_tft, (char*)pgm_read_word(&(table_MISC[0])));
  tft.drawString(buffer_tft, 0, 0, 2);

  Serial.begin(115200);

  MySignals.initSensorUART();
  MySignals.enableSensorUART(BLE);

  //Enable BLE module power -> bit6: 1
  bitSet(MySignals.expanderState, EXP_BLE_POWER);
  MySignals.expanderWrite(MySignals.expanderState);

  //Enable BLE UART flow control -> bit5: 0
  bitClear(MySignals.expanderState, EXP_BLE_FLOW_CONTROL);
  MySignals.expanderWrite(MySignals.expanderState);


  //Disable BLE module power -> bit6: 0
  bitClear(MySignals.expanderState, EXP_BLE_POWER);
  MySignals.expanderWrite(MySignals.expanderState);

  delay(500);

  //Enable BLE module power -> bit6: 1
  bitSet(MySignals.expanderState, EXP_BLE_POWER);
  MySignals.expanderWrite(MySignals.expanderState);
  delay(1000);

  MySignals_BLE.initialize_BLE_values();


  if (MySignals_BLE.initModule() == 1)
  {

    if (MySignals_BLE.sayHello() == 1)
    {
      //TFT message: "BLE init ok";
      strcpy_P((char*)buffer_tft, (char*)pgm_read_word(&(table_MISC[1])));
      tft.drawString(buffer_tft, 0, 15, 2);
    }
    else
    {
      //TFT message:"BLE init fail"
      strcpy_P((char*)buffer_tft, (char*)pgm_read_word(&(table_MISC[2])));
      tft.drawString(buffer_tft, 0, 15, 2);


      while (1)
      {
      };
    }
  }
  else
  {
    //TFT message: "BLE init fail"
    strcpy_P((char*)buffer_tft, (char*)pgm_read_word(&(table_MISC[2])));
    tft.drawString(buffer_tft, 0, 15, 2);

    while (1)
    {
    };
  }


}

void loop()
{
  available_temperature = MySignals_BLE.scanDevice(MAC_TEMPERATURE, 1000, TX_POWER_MAX);

  tft.drawString("Avail:", 0, 30, 2);
  if(available_temperature== (0||1))
  {
      tft.drawNumber(available_temperature, 110, 30, 2);
  }


  if (available_temperature == 1)
  {

    if (MySignals_BLE.connectDirect(MAC_TEMPERATURE) == 1)
    {
      connected_temperature = 1;


      tft.drawString("Connected", 0, 45, 2);


      delay(1000);


      char attributeData[2] =
      {
        0x01, 0x00
      };

      if (MySignals_BLE.attributeWrite(MySignals_BLE.connection_handle, TEMPERATURE_MEASURE_HANDLE, attributeData, 2) == 0)
      {
        unsigned long previous = millis();
        do
        {

          if (MySignals_BLE.waitEvent(1000) == BLE_EVENT_ATTCLIENT_ATTRIBUTE_VALUE)
          {

            if ((MySignals_BLE.event[9] == 2))
            {
              uint32_t tempVar = 0;
              uint8_t th = MySignals_BLE.event[10];
              uint8_t tm = MySignals_BLE.event[11];
              uint8_t tl = MySignals_BLE.event[12];
             
              tempVar = tl;
              tempVar <<= 8;
              tempVar = tm;
              tempVar <<= 8;
              tempVar = tempVar | th;

              temperature = (float) (tempVar / 100.00);
 
              tft.drawString("Temp:", 0, 75, 2);
              tft.drawFloat(temperature, 2, 100, 75, 2);
            }
          }
        }
        while ((connected_temperature == 1));
        
        connected_temperature = 0;
        
      }
      else
      {
        tft.drawString("Error subscribing", 0, 60, 2);
      }
    }
    else
    {
      connected_temperature = 0;

      tft.drawString("Not Connected", 0, 45, 2);
    }
  }
  else if (available_temperature == 0)
  {
    //Do nothing
  }

  delay(1000);
}


