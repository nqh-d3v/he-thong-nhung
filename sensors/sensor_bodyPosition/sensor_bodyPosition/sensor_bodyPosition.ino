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
  MySignals.initBodyPosition();

} 


void loop() 
{ 
  uint8_t position = MySignals.getBodyPosition(); 
  Serial.print("Current position : ");
  //print body position
  MySignals.printPosition(position); 
  delay(100);
   
  //print acc values
  MySignals.getAcceleration();
  
  // convert float to strintg
  char bufferAcc[50];
  char x_acc_string[10];  
  char y_acc_string[10];
  char z_acc_string[10];
  dtostrf (MySignals.x_data, 2, 2, x_acc_string); 
  dtostrf (MySignals.y_data, 2, 2, y_acc_string);
  dtostrf (MySignals.z_data, 2, 2, z_acc_string);
			
  // print the X Y Z acceleration
  sprintf (bufferAcc, "Acceleration: X= %s  Y= %s  Z= %s  ", x_acc_string, y_acc_string, z_acc_string);
  Serial.println(bufferAcc);
			
  delay(1000);
  
}






