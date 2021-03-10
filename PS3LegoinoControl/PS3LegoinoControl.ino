/* PS3Legoino Control
   Connects to a PS3 game pad and sends stick and button status via serial UART to a second ESP32 running DuploTrainControl.ino
   (C) 2021 by Frederik Holst
   
   Based on the awesome ESP32-PS3 library: https://github.com/jvpernis/esp32-ps3
*/

#include <Ps3Controller.h>

// Enter the Bluetooth-Address of your PS3 controller here
const char PS3Address[] = "08:6d:41:ba:7d:5f";

int8_t speed = 0;
int8_t old_speed = 0;
uint8_t button = 0;
uint8_t old_button = 0;
bool analog_control = false;

void notify()
{
  if(Ps3.event.button_down.cross) {
    Ps3.setRumble(100.0, 100);
    speed = 0;
  }
  if(Ps3.event.button_up.cross) {
    Ps3.setRumble(0.0);
  }
}

void onConnect(){
  Serial.println("Connected.");
}

void setup()
{
    Serial.begin(115200);
    Serial1.begin(115200, SERIAL_8N1, 17, 16);

    Serial.println("Starting PS3 Remote Control for Legoino...");

    Ps3.attach(notify);
    Ps3.attachOnConnect(onConnect);
    Ps3.begin(PS3Address);

    Serial.println("Ready.");
}

void loop() {
  if(!Ps3.isConnected()) return;
  
  if (abs(Ps3.data.analog.stick.ly) > 10 || abs(Ps3.data.analog.stick.ry) > 10) {
    analog_control = true;
  }
  if (analog_control == true) {
    if (abs(Ps3.data.analog.stick.ly)>abs(Ps3.data.analog.stick.ry)) {
      speed = Ps3.data.analog.stick.ly;
    } else {
      speed = Ps3.data.analog.stick.ry;
    }
    speed = -speed;
  }
  if (Ps3.data.button.up) {
    speed = speed+1;
    analog_control = false;
  }
  if (Ps3.data.button.down) {
    speed = speed-1;
    analog_control = false;
  }
  if (speed < -120) speed = -120;
  if (speed > 120) speed = 120;

  button = Ps3.data.button.triangle + \
           (Ps3.data.button.circle << 1) + \
           (Ps3.data.button.cross << 2) + \
           (Ps3.data.button.square << 3);

  if (old_speed != speed || old_button != button) {
    Serial.print(speed);
    Serial.print(", ");
    Serial.println(button);
    uint8_t sendArray[4] = {0};
    sendArray[0] = 0x7E;
    sendArray[1] = speed;
    sendArray[2] = button;
    sendArray[3] = 0x7F;
    Serial1.write(sendArray, 4);
    Serial1.flush();
    old_speed = speed;
    old_button = button;
  }
  delay(10);
}
