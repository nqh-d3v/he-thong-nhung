#include <SoftwareSerial.h>
#include <Wire.h>

SoftwareSerial mySerial(9, 10);
int x = 0;
void setup()
{
  mySerial.begin(9600);   // Setting the baud rate of GSM Module  
  Serial.begin(9600);    // Setting the baud rate of Serial Monitor (Arduino)
  delay(100);
}

void loop() {
  Wire.begin(9);
  Wire.onReceive(receiveEvent);
}
void receiveEvent( int bytes ) {
  Serial.println("Received");
  x = Wire.read();
  Wire.endTransmission();
  sendSMS(x);
}

void sendSMS(int x) {
  if (Serial.available()>0) {
    mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
    delay(1000);  // Delay of 1000 milli seconds or 1 second
    mySerial.println("AT+CMGS=\"+84384427258\"\r"); // Replace x with mobile number
    delay(1000);
    
    // 1 - 6 <=> [1 - 6] : Temp
    // 7 - 11 <=> [1 - 5] : SPO2
    // 12 - 15 <=> [1 - 4] : Pulse
    if (x > 0 && x <= 6) {
      mySerial.println("Temperature in level" + String(x));
    } else if ( x <= 11) {
      mySerial.println("SPO2 in level" + String(x - 6));
    } else if ( x <= 15) {
      mySerial.println("Pulse(bPm) in level" + String(x - 11));
    }
    delay(100);
    
     mySerial.println((char)26);// ASCII code of CTRL+Z
    delay(1000);
  }  
 if (mySerial.available()>0) {
   Serial.write(mySerial.read());
 }
}
