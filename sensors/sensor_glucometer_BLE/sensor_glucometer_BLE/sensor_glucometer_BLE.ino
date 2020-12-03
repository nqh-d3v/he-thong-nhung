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

#include <MySignals.h>
#include <MySignals_BLE.h>

#define GLUCO_HANDLE1 99
#define GLUCO_HANDLE2 83

// Write here the MAC address of BLE device to find
char MAC_GLUCO[14] = "187A93001030";

uint8_t available_gluco = 0;
uint8_t connected_gluco = 0;
uint8_t connection_handle_gluco = 0;

//!Struct to store data of the glucometer.
struct glucometerBLEDataVector
{
  uint16_t glucose;
  uint8_t info;
};

//!Vector to store the glucometer measures and dates.
glucometerBLEDataVector glucometerBLEData;

void setup()
{

  MySignals.begin();

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
      MySignals.println("BLE init ok");
    }
    else
    {
      MySignals.println("BLE init fail");

      while (1)
      {
      };
    }
  }
  else
  {
    MySignals.println("BLE init fail");

    while (1)
    {
    };
  }

}

void loop()
{
  available_gluco = MySignals_BLE.scanDevice(MAC_GLUCO, 1000, TX_POWER_MAX);

  MySignals.disableMuxUART();
  Serial.print(F("Gluco available:"));
  Serial.println(available_gluco);
  MySignals.enableMuxUART();


  if (available_gluco == 1)
  {

    if (MySignals_BLE.connectDirect(MAC_GLUCO) == 1)
    {
      MySignals.println("Connected");
      connected_gluco = 1;

      uint8_t gluco_subscribe_message[2] = { 0x01 , 0x00 };
      delay(200);

      MySignals_BLE.attributeWrite(MySignals_BLE.connection_handle, GLUCO_HANDLE1, gluco_subscribe_message, 2);
      MySignals_BLE.attributeWrite(MySignals_BLE.connection_handle, GLUCO_HANDLE2, gluco_subscribe_message, 2);
      
      delay(200);
      MySignals.println("Insert blood stripe (20s)");

      unsigned long previous = millis();
      do
      {
        if (MySignals_BLE.waitEvent(1000) == BLE_EVENT_ATTCLIENT_ATTRIBUTE_VALUE)
        {
          MySignals.disableMuxUART();
          Serial.print(F("Glucose(mg/dl):"));

          if (MySignals_BLE.event[8] == 0x0c)
          {
            uint8_t gh = MySignals_BLE.event[12] - 48;
            uint8_t gl = MySignals_BLE.event[13] - 48;
            glucometerBLEData.glucose = (gh * 10) + gl;
            glucometerBLEData.info = 0;

            Serial.println(glucometerBLEData.glucose);
          }
          if (MySignals_BLE.event[8] == 0x0d)
          {

            uint8_t gh = MySignals_BLE.event[12] - 48;
            uint8_t gm = MySignals_BLE.event[13] - 48;
            uint8_t gl = MySignals_BLE.event[14] - 48;
            glucometerBLEData.glucose = (gh * 100) + (gm * 10) + gl;
            glucometerBLEData.info = 0;

            Serial.println(glucometerBLEData.glucose);
          }
          if (MySignals_BLE.event[8] == 0x0e)
          {
            if (MySignals_BLE.event[12] == 0x4c)
            {
              glucometerBLEData.glucose = 0;
              glucometerBLEData.info = 0xAA;
              
              Serial.println(F("Low glucose"));
            }
            else if (MySignals_BLE.event[12] == 0x48)
            {
              glucometerBLEData.glucose = 360;
              glucometerBLEData.info = 0xBB;
              
              Serial.println(F("High glucose"));
             
            }

          }

          MySignals_BLE.disconnect(MySignals_BLE.connection_handle);
          connected_gluco = 0;
        }

      }
      while ((connected_gluco == 1) && ((millis() - previous) < 20000)); //Timeout 20 seconds

      connected_gluco = 0;



    }
    else
    {
      connected_gluco = 0;
      MySignals.println("Not Connected");
    }


  }
  else if (available_gluco == 0)
  {
    //Do nothing
  }
  else
  {
    MySignals_BLE.hardwareReset();
    MySignals_BLE.initialize_BLE_values();
    delay(100);

  }
  delay(1000);
}


