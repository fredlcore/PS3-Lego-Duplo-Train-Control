/**
 * A Legoino example to control a Duplo Train hub
 * Based on the awesome Legoino library: https://github.com/corneliusmunz/legoino
 *
 * Start/reset the ESP32 and your train and the serial monitor of the 
 * Arduino IDE. Once the train is connected, you can control it via the
 * following keys:
 * 
 * "1...9" - Select active train
 * "+"     - Increase speed
 * "-"     - Decrease speed
 * " "     - Stop the train
 * "X"     - Turn off the train
 * 
 * See PS3LegoinoControl for how to control the train via a PS3 controller
 * 
 * (c) Copyright 2021 - Frederik Holst
 * Released under MIT License
 * 
 */

#include "Lpf2Hub.h"

// create hub instances
#define HUBS 3
Lpf2Hub* myHubs[HUBS];
bool activeHubs[HUBS] = { false };
int8_t activeHub = -1;
int8_t speed[HUBS] = { 0 };
int8_t parsed_speed[HUBS] = { 0 };
int8_t saved_speed[HUBS] = { 0 };

int8_t fillup_stage[HUBS] = { 0 };
unsigned long fillup_timer[HUBS] = { 0 };
bool tile_override[HUBS] = { 0 };
unsigned long tile_override_timer[HUBS] = { 0 };

// Color LedColors[] = {BLACK, PINK, PURPLE, BLUE, LIGHTBLUE, CYAN, GREEN, YELLOW, ORANGE, RED, WHITE, NONE};
// byte Sounds[] = {(byte)DuploTrainBaseSound::BRAKE, (byte)DuploTrainBaseSound::STATION_DEPARTURE, (byte)DuploTrainBaseSound::WATER_REFILL, (byte)DuploTrainBaseSound::HORN, (byte)DuploTrainBaseSound::STEAM};

void colorSensorCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData) {
  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  static unsigned long last_color_change = millis();
  static int8_t last_color[HUBS] = { 0 };

  if (deviceType == DeviceType::DUPLO_TRAIN_BASE_COLOR_SENSOR) {
    int8_t callback_hub = -1;
    for (int8_t i=0; i<HUBS; i++) {
      if (myHubs[i] == myHub) {
        callback_hub = i;
      }
    }
    int scanned_color = myHub->parseColor(pData);
    Serial.print("Scanned color: ");
    Serial.println(COLOR_STRING[scanned_color]);

    if (millis() - last_color_change > 3000 || (scanned_color != last_color[callback_hub] && scanned_color < 255)) {
      last_color[callback_hub] = scanned_color;
      if (scanned_color == (byte)RED) {
        // Red tile (brake): stop and play brake sound
        speed[callback_hub] = 0;
        setSound(DuploTrainBaseSound::BRAKE, myHub);
        tile_override[callback_hub] = true;
        tile_override_timer[callback_hub] = millis();
      } else if (scanned_color == (byte)BLUE) {
        // Blue tile (water): stop, play steam sound once, play fillup sound thrice, resume original speed
        fillup_stage[callback_hub] = 1;
        saved_speed[callback_hub] = speed[callback_hub];
        fillup_timer[callback_hub] = millis();
        speed[callback_hub] = 0;
      } else if (scanned_color == (byte)YELLOW) {
        // Yellow tile (horn): play horn sound
        setSound(DuploTrainBaseSound::HORN, myHub);
      } else if (scanned_color == (byte)GREEN) {
        // Green tile (reverse): play station departure sound, revert speed
        speed[callback_hub] = -speed[callback_hub];
        setSound(DuploTrainBaseSound::STATION_DEPARTURE, myHub);
        tile_override[callback_hub] = true;
        tile_override_timer[callback_hub] = millis();
      } else if (scanned_color == (byte)WHITE) {
        // White tile (light): Change light to white
        setColor(WHITE, myHub);
      }
      last_color_change = millis();
    }
  }
}

void speedometerSensorCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData) {
  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  static int8_t old_parsed_speed[HUBS] = { 0 };


  if (deviceType == DeviceType::DUPLO_TRAIN_BASE_SPEEDOMETER) {
    for (int8_t i=0; i<HUBS; i++) {
      if (myHubs[i] == myHub) {
        old_parsed_speed[i] = parsed_speed[i];
        parsed_speed[i] = myHub->parseSpeedometer(pData);
        if (parsed_speed[i] == 0 && abs(old_parsed_speed[i]) > 0) {
          speed[i] = 0;
        }
      }
    }
  }
}

// Change speed
void move(int8_t speed, Lpf2Hub *hub) {
  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  speed = (speed / 3) * 2;
  if (abs(speed) < 10) speed = 0;
  if (speed < -78) speed = -78;
  if (speed > 79) speed = 79;
  if (speed == 0 && parsed_speed[activeHub] == 0) {} else {
    myHub->setBasicMotorSpeed((byte)DuploTrainHubPort::MOTOR, speed);
  }
//  delay(50);
}

// Play sound
void setSound(DuploTrainBaseSound sound, Lpf2Hub *hub) {
  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  Serial.print("Sound: ");
  Serial.println((byte)sound);
  for (int8_t x=0; x<10; x++) {
    myHub->playSound((byte)sound);
  }
}

// Change color
void setColor(Color color, Lpf2Hub *hub) {
  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  Serial.print("Color: ");
  Serial.println(COLOR_STRING[color]);
  for (int8_t x=0; x<10; x++) {
    myHub->setLedColor(color);
  }
}

int8_t setActiveHub(char c) {
  int8_t hub = -1;
  c = c - '0' - 1;
  for (int8_t i=0; i<HUBS; i++) {
    if (activeHubs[i] == true) {
      hub = i;
      if (hub == c) break;
    }
  }
  return hub;
}

int8_t nextHub(int8_t hub) {
  setColor(CYAN, myHubs[hub]);
  int8_t i = 0;
  for (int8_t x=0; x<HUBS; x++) {
    i++;
    if (hub + i >= HUBS) i = -hub;
    if (activeHubs[hub+i] == true) {
      delay(200);
      setColor(RED, myHubs[hub+i]);
      return hub+i;
    }
  }
  return -1;
}

void shutdown(int8_t hub) {
  myHubs[hub]->shutDownHub(); 
  delay(1000);
}

// If train moves over a blue tile ("fill up"), then first play steam sound, wait, play fill-up sound, resume original speed.
void processFillup(int8_t hub) {  
  delay(50);
  if (fillup_stage[hub] == 1) {
    setSound(DuploTrainBaseSound::STEAM, myHubs[hub]);
    fillup_stage[hub] = 2;
  }
  if (fillup_stage[hub] >= 2 && fillup_stage[hub] <= 4 && millis() - fillup_timer[hub] > 1300) {
    fillup_timer[hub] = millis();
    setSound(DuploTrainBaseSound::WATER_REFILL, myHubs[hub]);
    fillup_stage[hub] += 1;
  }
  if (fillup_stage[hub] == 5 && millis() - fillup_timer[hub] > 2000) {
    speed[hub] = saved_speed[hub];
    fillup_stage[hub] = 0;
  }
}

void setup() {
  delay(3000);
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 16, 17);
  Serial.print("Initializing...");
  for (int8_t x=0; x<HUBS; x++) {
    myHubs[x] = new Lpf2Hub;
    myHubs[x]->init();
  }
  Serial.println("Done");
  Serial.println("After connecting, you can control your train with these keys:\r\n\
    \"1...9\" - Select active hub\r\n\
    \"+\"     - Increase speed\r\n\
    \"-\"     - Decrease speed\r\n\
    \" \"     - Stop the train\r\n\
    \"X\"     - Turn off train");
}

// main loop
void loop() {

  static char c = 0;
  static int8_t analog_speed = 0;
  static bool analog_control[HUBS] = { 0 };
  static uint8_t button = 0;
  static uint8_t old_button = 0;
  static unsigned long idle_timer = millis();

  // connect flow
  for (int8_t hub=0; hub<HUBS; hub++) {
    Lpf2Hub* myHub = myHubs[hub];
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
        activeHubs[hub] = true;
        if (activeHub == -1) activeHub = hub;
        delay(200);
        // connect color sensor and activate it for updates
        myHub->activatePortDevice((byte)DuploTrainHubPort::SPEEDOMETER, speedometerSensorCallback);
        delay(200);
        // connect speed sensor and activate it for updates
        myHub->activatePortDevice((byte)DuploTrainHubPort::COLOR, colorSensorCallback);
        delay(200);
        setSound(DuploTrainBaseSound::HORN, myHub);
      } else {
        Serial.println("Failed to connect to Duplo Hub");
        myHub->init();
      }
    }
  }

  for (int8_t hub=0; hub<HUBS; hub++) {

    Lpf2Hub* myHub = myHubs[hub];
    if (myHub->isConnected()) {

      // Read keys from serial input
      if (Serial.available()) {
        // Read key from serial
        c = Serial.read();
        // Discard control characters, otherwise print character
        if (c > 31) Serial.println(c);
        idle_timer = millis();
      }

      // Read buttons and movements from PS3 controller via Serial1
      while (Serial1.available()) {
        while (Serial1.peek() != 0x7E && Serial1.available()) Serial.println(Serial1.read());
        Serial1.read();
        analog_speed = (int8_t)Serial1.read();
        if (abs(analog_speed) > 10) analog_control[activeHub] = true;
        button = Serial1.read();
        Serial1.read();
        Serial.print("Speed: ");
        Serial.print(analog_speed);
        Serial.print("\tButton: ");
        Serial.println(button);
        idle_timer = millis();
      }

      // Process keys received
      switch (c) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': activeHub = setActiveHub(c); break;
        case '+': speed[activeHub]+=10; analog_control[activeHub] = false; break;
        case '-': speed[activeHub]-=10; analog_control[activeHub] = false; break;
        case ' ': speed[activeHub] = 0; break;
        case 'X': shutdown(activeHub); break;
        default: break;
      }
      c = 0;      // Clear keypress variable

      // Process PS3 inputs
      if (button != old_button || button >= 64) { // Normally, only react if there has been a change, only for digital keypad up/down react always to allow for smooth increase of speed
        old_button = button;
        switch (button) {
          case 1: setSound(DuploTrainBaseSound::STATION_DEPARTURE, myHubs[activeHub]); break;
          case 2: setSound(DuploTrainBaseSound::STEAM, myHubs[activeHub]); break;
          case 4: speed[activeHub] = 0; setSound(DuploTrainBaseSound::BRAKE, myHubs[activeHub]); break;
          case 8: setSound(DuploTrainBaseSound::HORN, myHubs[activeHub]); break;
          case 16: activeHub = nextHub(activeHub); break; // Select button pressed, switch active hub
          case 32: speed[activeHub] = 40; break;          // Start button pressed, set speed to 50% forward
          case 48: shutdown(activeHub); break;            // Start and Select button pressed, turn off train
          case 64: speed[activeHub] += 1; if (speed[activeHub] < 10 && speed[activeHub] > 0) speed[activeHub] = 40; analog_control[activeHub] = false; break;   // Up button pressed, increase speed
          case 128: speed[activeHub] -= 1; if (speed[activeHub] > -10 && speed[activeHub] <0) speed[activeHub] = -40; analog_control[activeHub] = false; break;  // Down button pressed, decrease speed
          default: break;
        }
        idle_timer = millis();
      }
      if (analog_control[activeHub] == true && fillup_stage[hub] == 0 && tile_override[hub] == false) speed[activeHub] = analog_speed;
      if (speed[activeHub]>120) speed[activeHub]=120;
      if (speed[activeHub]<-120) speed[activeHub]=-120;
      if (abs(speed[activeHub])<2) speed[activeHub]=0;
      move(speed[hub], myHub);

      if (fillup_stage[hub] > 0) processFillup(hub);
      if (tile_override[hub] == true && millis() - tile_override_timer[hub] > 3000) tile_override[hub] = false;  // Tile action overrides analog stick for 3 seconds
    } else {
      if (myHub->isScanning() == false && myHub->isConnecting() == false && myHub->isConnected() == false) {
        activeHubs[hub] = false;
        if (activeHub == hub) {
          Serial.print("Hub ");
          Serial.print(hub);
          Serial.print(" no longer active, assigning hub ");
          activeHub = setActiveHub(0);
          Serial.print(activeHub);
          Serial.println(" as new active hub.");
        }
        Serial.print("Start new scan on slot ");
        Serial.println(hub);
        myHub->init();
      }
    }
  }

  if (millis() - idle_timer > 300000 && activeHub > -1) {
    Serial.println("Idle time exceeded, shutting down hubs...");
    for (int8_t i=0; i<HUBS; i++) {
      if (myHubs[i]->isConnected()) {
        Serial.println(i);
        shutdown(i);
      }
    }
    idle_timer = millis();
  }

  delay(10);
} // End of loop
