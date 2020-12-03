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
#include <Wire.h>
#include "SPI.h"


void setup()
{
  Serial.begin(115200);
  MySignals.begin();
  MySignals.initSensorUART();
  MySignals.enableSensorUART(GLUCOMETER);

  delay(1000);

  MySignals.getGlucose();
  
  Serial.begin(115200);
  MySignals.disableMuxUART();
  for (uint8_t i = 0; i < MySignals.glucoseLength; i++)
  {
    Serial.print(F("measure number:"));
    Serial.println(i + 1);
    Serial.print(F("year:"));
    Serial.println(MySignals.glucometerData[i].year);
    Serial.print(F("month:"));
    Serial.println(MySignals.glucometerData[i].month);
    Serial.print(F("day:"));
    Serial.println(MySignals.glucometerData[i].day);
    Serial.print(F("hour:"));
    Serial.println(MySignals.glucometerData[i].hour);
    Serial.print(F("minutes:"));
    Serial.println(MySignals.glucometerData[i].minutes);
    Serial.print(F("glucose:"));
    Serial.println(MySignals.glucometerData[i].glucose);
    Serial.print(F("meridian:"));
    Serial.println(MySignals.glucometerData[i].meridian);
    Serial.println();
  }
  MySignals.enableMuxUART();

}

void loop()
{


}
