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


// Write here the MAC address of BLE device to find
char MAC_BP[14] = "A4D57812A409";


uint8_t available_bp = 0;
uint8_t connected_bp = 0;
uint8_t connection_handle_bp = 0;

#define BP_HANDLE 18



//!Struct to store data of the glucometer.
struct bloodPressureBLEDataVector
{
  uint16_t systolic;
  uint16_t diastolic;
  uint16_t pulse;
};

//!Vector to store the glucometer measures and dates.
bloodPressureBLEDataVector bloodPressureBLEData;

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
  available_bp = MySignals_BLE.scanDevice(MAC_BP, 1000, TX_POWER_MAX);

  MySignals.disableMuxUART();
  Serial.print("BP available:");
  Serial.println(available_bp);
  MySignals.enableMuxUART();


  if (available_bp == 1)
  {

    if (MySignals_BLE.connectDirect(MAC_BP) == 1)
    {
      MySignals.println("Connected");
      
      connected_bp = 1;
      delay(8000);
      //To subscribe the BP measure write an ASCII letter "e"
      if (MySignals_BLE.attributeWrite(MySignals_BLE.connection_handle,  BP_HANDLE, "e", 1) == 0)
      {

       MySignals.println("Subscribed");

        unsigned long previous = millis();
        do
        {

          if (MySignals_BLE.waitEvent(1000) == BLE_EVENT_ATTCLIENT_ATTRIBUTE_VALUE)
          {
            //Search letter "g" in ASCII
            if (MySignals_BLE.event[9] == 0x67)
            {

              if (MySignals_BLE.event[10] == 0x2f)
              {
                //Ojo esto esta mal en la guia de comandos

                //Primero da la alta -> sistolica
                uint8_t sh = MySignals_BLE.event[11] - 48;
                uint8_t sm = MySignals_BLE.event[12] - 48;
                uint8_t sl = MySignals_BLE.event[13] - 48;
                bloodPressureBLEData.systolic = (sh * 100) + (sm * 10) + sl;

                //Primero da la baja -> diastolica
                uint8_t dh = MySignals_BLE.event[15] - 48;
                uint8_t dm = MySignals_BLE.event[16] - 48;
                uint8_t dl = MySignals_BLE.event[17] - 48;
                bloodPressureBLEData.diastolic = (dh * 100) + (dm * 10) + dl;

                uint8_t ph = MySignals_BLE.event[19] - 48;
                uint8_t pm = MySignals_BLE.event[20] - 48;
                uint8_t pl = MySignals_BLE.event[21] - 48;
                bloodPressureBLEData.pulse = (ph * 100) + (pm * 10) + pl;


                MySignals.disableMuxUART();
                Serial.print(F("Systolic: "));
                Serial.println(bloodPressureBLEData.systolic);

                Serial.print(F("Diastolic: "));
                Serial.println( bloodPressureBLEData.diastolic);

                Serial.print(F("Pulse/min: "));
                Serial.println(bloodPressureBLEData.pulse);
                Serial.println(F("Disconnected from device"));

                MySignals.enableMuxUART();
                
                connected_bp = 0;

              }
            }
          }
        }
        while ((connected_bp == 1) && ((millis() - previous) < 40000));


        MySignals_BLE.attributeWrite(MySignals_BLE.connection_handle,  BP_HANDLE, "i" , 1);
        delay(100);


        MySignals_BLE.disconnect(MySignals_BLE.connection_handle);

        connected_bp = 0;

      }
      else
      {
        MySignals.println("Error subscribing");
      }

    }
    else
    {
      connected_bp = 0;
      MySignals.println("Not Connected");
    }
  }
  else if (available_bp == 0)
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


