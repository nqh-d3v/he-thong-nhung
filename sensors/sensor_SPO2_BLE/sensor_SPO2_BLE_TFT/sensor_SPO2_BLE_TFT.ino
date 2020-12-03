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
char MAC_SPO2[14] = "00A0500E1823";


uint8_t available_spo2 = 0;
uint8_t connected_spo2 = 0;
uint8_t connection_handle_spo2 = 0;
uint8_t pulse_spo2 = 0;
uint8_t spo2 = 0;

#define SPO2_HANDLE 15

char buffer_tft[30];

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
  available_spo2 = MySignals_BLE.scanDevice(MAC_SPO2, 1000, TX_POWER_MAX);

  tft.drawString("SPO2 available:", 0, 30, 2);
  tft.drawNumber(available_spo2, 110, 30, 2);



  if (available_spo2 == 1)
  {

    if (MySignals_BLE.connectDirect(MAC_SPO2) == 1)
    {
      connected_spo2 = 1;
      connection_handle_spo2 = MySignals_BLE.connection_handle;

      tft.drawString("Connected    ", 0, 45, 2);


      delay(6000);

      //To subscribe the spo2 measure write "1" in SPO2_HANDLE
      char attributeData[1] =
      {
        0x01
      };

      if (MySignals_BLE.attributeWrite(connection_handle_spo2, SPO2_HANDLE, attributeData, 1) == 0)
      {
        tft.drawString("Subscribed", 0, 60, 2);

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

              sprintf(buffer_tft, "Pulse: %d ppm / SPO2: %d", pulse_spo2, spo2);
              tft.drawString(buffer_tft, 0, 75, 2);



              uint16_t errorCode = MySignals_BLE.disconnect(connection_handle_spo2);
              tft.drawString("Disconnected", 0, 45, 2);

              connected_spo2 = 0;

            }
          }
        }
        while ((connected_spo2 == 1) && ((millis() - previous) < 10000));

        connected_spo2 = 0;

      }
      else
      {
        tft.drawString("Error subscribing", 0, 60, 2);
      }
    }
    else
    {
      connected_spo2 = 0;

      tft.drawString("Not Connected", 0, 45, 2);
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


