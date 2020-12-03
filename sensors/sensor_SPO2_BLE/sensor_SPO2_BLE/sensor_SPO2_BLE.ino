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
char MAC_SPO2[14] = "00A0500E1823";


uint8_t available_spo2 = 0;
uint8_t connected_spo2 = 0;
uint8_t connection_handle_spo2 = 0;
uint8_t pulse_spo2 = 0;
uint8_t spo2 = 0;

#define SPO2_HANDLE 15

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
  available_spo2 = MySignals_BLE.scanDevice(MAC_SPO2, 1000, TX_POWER_MAX);

  MySignals.disableMuxUART();
  Serial.print("SPO2 available:");
  Serial.println(available_spo2);
  MySignals.enableMuxUART();

  if (available_spo2 == 1)
  {
    MySignals.disableMuxUART();
    Serial.println("SPO2 found.Connecting");
    MySignals.enableMuxUART();


    if (MySignals_BLE.connectDirect(MAC_SPO2) == 1)
    {
      connected_spo2 = 1;
      connection_handle_spo2 = MySignals_BLE.connection_handle;

      MySignals.println("Connected");

      delay(6000);

      //To subscribe the spo2 measure write "1" in SPO2_HANDLE
      char attributeData[1] =
      {
        0x01
      };

      if (MySignals_BLE.attributeWrite(connection_handle_spo2, SPO2_HANDLE, attributeData, 1) == 0)
      {
        unsigned long previous = millis();
        do
        {
          if (MySignals_BLE.waitEvent(1000) == BLE_EVENT_ATTCLIENT_ATTRIBUTE_VALUE)
          {


            char attributeData[1] = {  0x00 };

            MySignals_BLE.attributeWrite(connection_handle_spo2, SPO2_HANDLE, attributeData , 1);

            uint8_t pulse_low = MySignals_BLE.event[12];
            pulse_low &= 0b01111111;

            uint8_t pulse_high = MySignals_BLE.event[11];
            pulse_high &= 0b01000000;

            if (pulse_high == 0)
            {
              pulse_spo2 = pulse_low;
            }

            if (pulse_high == 0b01000000)
            {
              pulse_spo2 = pulse_low + 0b10000000;
            }

            spo2 = MySignals_BLE.event[13];
            spo2 &= 0b01111111;

            if ((pulse_spo2 >= 25) && (pulse_spo2 <= 250)
                && (pulse_spo2 >= 35) && (pulse_spo2 <= 100))
            {
              MySignals.disableMuxUART();

              Serial.println();
              Serial.print(F("SpO2: "));
              Serial.print(spo2);
              Serial.print(F("%  "));
              Serial.print(F("Pulse: "));
              Serial.print(pulse_spo2);
              Serial.println(F("ppm  "));

              uint16_t errorCode = MySignals_BLE.disconnect(connection_handle_spo2);

              Serial.print(F("Disconnecting error code: "));
              Serial.println(errorCode, HEX);

              MySignals.enableMuxUART();
              connected_spo2 = 0;

            }
          }
        }
        while ((connected_spo2 == 1) && ((millis() - previous) < 10000));

        connected_spo2 = 0;

      }
      else
      {
        MySignals.println("Error subscribing");
      }
    }
    else
    {
      connected_spo2 = 0;

      MySignals.println("Not Connected");
    }
  }
  else if (available_spo2 == 0)
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


