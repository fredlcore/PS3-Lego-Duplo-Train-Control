/**
 * A Legoino example to control a Duplo Train hub
 * Based on the awesome Legoino library: https://github.com/jvpernis/esp32-ps3
 *
 * Start/reset the ESP32 and your train and the serial monitor of the 
 * Arduin IDE. Once the train is connected, you can control it via the
 * following keys:
 * 
 * "+" - Increase speed
 * "-" - Decrease speed
 * " " - Stop the train
 * "S" - Play next sound
 * "C" - Change to next color (color name will be displayed in serial monitor)
 * 
 * (c) Copyright 2021 - Fredrik Holst
 * Released under MIT License
 * 
 */

#include "Lpf2Hub.h"

// create a hub instance
#define HUBS 3
Lpf2Hub* myHubs[HUBS];
Lpf2Hub* myHub;

byte motorPort = (byte)DuploTrainHubPort::MOTOR;
int speed = 0;
int parsed_speed = 0;
int button = 0;
int old_button = 0;
int color = 0;
int sound = 0;
int tone = 0;

Color LedColors[] = {BLACK, PINK, PURPLE, BLUE, LIGHTBLUE, CYAN, GREEN, YELLOW, ORANGE, RED, WHITE, NONE};
byte Sounds[] = {(byte)DuploTrainBaseSound::BRAKE, (byte)DuploTrainBaseSound::STATION_DEPARTURE, (byte)DuploTrainBaseSound::WATER_REFILL, (byte)DuploTrainBaseSound::HORN, (byte)DuploTrainBaseSound::STEAM};

char c = 0;

void colorSensorCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Lpf2Hub *myHub = (Lpf2Hub *)hub;

  if (deviceType == DeviceType::DUPLO_TRAIN_BASE_COLOR_SENSOR) {
    int scanned_color = myHub->parseColor(pData);
    Serial.print("Scanned color: ");
    Serial.println(COLOR_STRING[scanned_color]);
    myHub->setLedColor((Color)scanned_color);
    if (scanned_color == (byte)RED) {
      myHub->playSound((byte)DuploTrainBaseSound::BRAKE);
    } else if (scanned_color == (byte)BLUE) {
      myHub->playSound((byte)DuploTrainBaseSound::WATER_REFILL);
    } else if (scanned_color == (byte)YELLOW) {
      myHub->playSound((byte)DuploTrainBaseSound::HORN);
    }
  }
}

void speedometerSensorCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData) {
  Lpf2Hub *myHub = (Lpf2Hub *)hub;

  if (deviceType == DeviceType::DUPLO_TRAIN_BASE_SPEEDOMETER) {
    parsed_speed = myHub->parseSpeedometer(pData);
/*
    Serial.print("Speed: ");
    Serial.println(parsed_speed);
*/
  }
}

// Change speed (keys pressed: + or -)
void move(int speed, Lpf2Hub *hub) {
  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  if (abs(speed) < 10) speed = 0;
    if (speed == 0 && parsed_speed == 0) {} else {
//    for (int x=0; x<10; x++) {
      myHub->setBasicMotorSpeed(motorPort, speed);
//    }
    }
    delay(1);
}

// Change color (key pressed: C)
void setColor(byte color, Lpf2Hub *hub) {
  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  Serial.print("Color: ");
  Serial.println(COLOR_STRING[color]);
  for (int x=0; x<10; x++) {
    myHub->setLedColor(LedColors[color]);
  }
}

// Play sound (key pressed: S)
void setSound(byte sound, Lpf2Hub *hub) {
  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  Serial.print("Sound: ");
  Serial.println(sound);
  for (int x=0; x<10; x++) {
    myHub->playSound(Sounds[sound]);
  }
}

// Play tone (key pressed: T)
void setTone(byte tone, Lpf2Hub *hub) {
  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  Serial.print("Tone: ");
  Serial.println(tone);
  for (int x=0; x<10; x++) {
    myHub->playTone(tone);
  }
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 16, 17);
  Serial.print("Initializing...");
//  myHub.init("18:04:ed:ea:4d:67");  // Dampflok
//  myHub.init("a4:da:32:04:ef:7a");  // Güterzug
  for (int x=0; x<HUBS; x++) {
    myHubs[x] = new Lpf2Hub;
    myHubs[x]->init();
  }
  Serial.println("Done");
  Serial.println("After connecting, you can control your train with these keys:\r\n\
    \"+\" - Increase speed\r\n\
    \"-\" - Decrease speed\r\n\
    \" \" - Stop the train\r\n\
    \"S\" - Play next sound\r\n\
    \"T\" - Play next tone\r\n\
    \"C\" - Change to next color (color name will be displayed in serial monitor)");
}

// main loop
void loop() {
  // connect flow
  for (int hub=0; hub<HUBS; hub++) {
    myHub = myHubs[hub];
    if (myHub->isConnecting()) {
      Serial.print("Connecting slot ");
      Serial.print(hub);
      Serial.println("...");
      myHub->connectHub();
      if (myHub->isConnected()) {
        Serial.println("Connected to Duplo Hub");
        Serial.print("Hub address: ");
        Serial.println(myHub->getHubAddress().toString().c_str());
        Serial.print("Hub name: ");
        Serial.println(myHub->getHubName().c_str());
        delay(200);
        // connect color sensor and activate it for updates
        myHub->activatePortDevice((byte)DuploTrainHubPort::SPEEDOMETER, speedometerSensorCallback);
        delay(200);
        // connect speed sensor and activate it for updates
        myHub->activatePortDevice((byte)DuploTrainHubPort::COLOR, colorSensorCallback);
        delay(200);
      } else {
        Serial.println("Failed to connect to Duplo Hub");
      }
    }
  }

  for (int hub=0; hub<HUBS; hub++) {
    myHub = myHubs[hub];
    if (myHub->isConnected()) {
      if (Serial.available()) {
        // Read key from serial
        c = Serial.read();
        // Discard control characters, otherwise print character
        if (c > 31) Serial.println(c);
      }

      while (Serial1.available()) {
        while (Serial1.peek() != 0x7E && Serial1.available()) Serial.println(Serial1.read());
        Serial1.read();
        speed = (int8_t)Serial1.read();
        button = Serial1.read();
        Serial1.read();
        Serial.print("Speed: ");
        Serial.print(speed);
        Serial.print("\tButton: ");
        Serial.println(button);
      }

      switch (c) {
        case '+': speed=speed+10; if (speed>100) speed=100; break;
        case '-': speed=speed-10; if (speed<-100) speed=-100; break;
        case ' ': speed = 0; break;
        case 'C': color=color+1; if (color>10) color=0; setColor(color, myHub); break;
        case 'S': sound=sound+1; if (sound>4) sound=0; setSound(sound, myHub); break;
        case 'T': tone=tone+1; if (tone>10) tone=0; setTone(tone, myHub); break;
        default: break;
      }
      move(speed, myHub);
      if (button != old_button) {
        old_button = button;
        switch (button) {
          case 1: setSound(1, myHub); break;
          case 2: setSound(4, myHub); break;
          case 4: setSound(0, myHub); break;
          case 8: setSound(3, myHub); break;
          default: break;
        }
      }
      c = 0;      // Clear keypress variable
    }
  }
  delay(10);
} // End of loop
