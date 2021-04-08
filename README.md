# PS3-Lego-Duplo-Train-Control
Control a Lego Duplo train with a PS3 game pad

*Disclaimer*: LEGO® is a trademark of the LEGO Group of companies which does not sponsor, authorize or endorse this project.

This project enables you to use your PS3 game pad together with two ESP32 boards to control one or more Lego Duplo trains (10874 / 10875) using the analog sticks, the D-pad and the symbol buttons. It is based on the two awesome libraries [Legoino](https://github.com/corneliusmunz/legoino) and [ESP32-PS3](https://github.com/jvpernis/esp32-ps3) which you need to install through the Arduino IDE prior to proceeding.  
  
Since both libraries use different Bluetooth libraries, it is unfortunately currently still necessary to separate the PS3 and the Lego connection. Therefore, two ESP32 microcontroller boards are necessary for the two functions. However, you can easily stack two boards on top of each other and only one board needs to be powered. If you don't have a PS3 controller, you can also control your train(s) via a terminal from your computer. In that case, you only need one ESP32.

## Configuration

### PS3LegoinoControl.ino
This program takes care of the connection with the PS3 controller. To be able to pair the controller with the ESP32 board, you need to find out the Bluetooth address stored inside the controller. There are several ways you can do this and also depend on your PC's operating system. Google for "SixAxis Tool" (Windows) or "SixAxisPairer" (Mac) as a start.  
Once you have found out the controller's Bluetooth address, enter it in the top of the `PS3LegoinoControl.ino` file in the variable `PS3Address`.

### DuploTrainControl.ino
Here you only need to adjust the number of hubs (i.e. trains) you want the ESP32 to set up. It defaults to three. It can be increased to nine if you make some changes in the NimBLE library, see [here]https://github.com/corneliusmunz/legoino#connection-to-more-than-3-hubs. If you have less than three trains, you can decrease the number in `#define HUBS 3` which will make the system minimally more responsive and less energy-consuming. 

## Installation
* Take the two ESP32 boards and flash `PSLegoinoControl.ino` on one board and `DuploTrainControl.ino` on the other. It doesn't matter which is which. 
* Then connect 5V/VIN, GND and GPIOs 16 and 17 between both boards in a 1:1 way, no need to cross-wire pins 16 and 17. When using the Wemos-style ESP32 boards, you can stack the two boards 1:1 on top of each other using the two inner pin rows.
* If you still want to be able to use the serial monitor when both boards are plugged together, do not connect the RXD/TXD pins.

## Getting started
* Power one of the two ESP32 boards via one of their Micro-USB ports.
* Turn on the Lego Duplo train(s)
* You should hear a sound from each train once it is connected.
* Press the "PS3" button on the PS3 controller. You should see the LEDs on the front starting to flash during pairing and one LED after pairing is complete. To check if pairing is successful, you can press the "X" button and the controller should react with a short vibration.

## Controlling the train via PS3 controller
Currently, the following features are supported:
* Analog sticks: Move up and down to gradually make the train go forward/backwards.
* D-Pad: Press up and down to control the train speed going forward/backward. In contrast to the analog stick, the speed will be maintained if you release the buttons.
* Cross button: Stop the train and play "brake" sound effect.
* Circle button: Play "steam" sound effect.
* Square button: Play "horn" sound effect.
* Triangle button: Play "station" sound effect (new!).
* Select button: Switch control between trains (if you have more than one).
* Select + Start button: Turn off current train.

You can also control the train by connecting the ESP32 to your computer and start a terminal program. Upon startup (such as reset), a menu will display the key commands.

## Known problems
* Some trains and ESP32 boards seem to be working better with each other than others and sometimes commands get "lost". I have tried to circumvent this for the speed commands as much as possible, but for the sound effects sometimes playing a sound gets skipped.

## Use PS3 game pad with other Legoino supported devices
This should be possible without too much effort. You can use `PS3LegoinoControl.ino` straight away and just rewrite `DuploTrainControl.ino` to fit your needs based on the examples in the Legoino project. The variables `speed` and `button` in `DuploTrainControl.ino` contain the "speed" (based on the D-pad or analog sticks) and the buttons pressed and you can just call other functions of the Legoino framework to make it work with other Lego devices supported by Legoino. So far the following controls are transmitted to `DuploTrainControl.ino`: Analog stick up/down, D-pad up/down, Start, Select and symbol buttons. To change the controls used, you have to adjust `PS3LegoinoControl.ino`.

## ToDo
* ~Deal with color detection: Currently, the sensor is over-sensitive and any change in floor texture results in a color detection action, therefore currently disabled.~ With new Legoino library version, this has been solved.
* ~Remotely turn off the train through the PS3 controller~ Can be done now by pressing Start and Select button.
* ~Use buttons to change the train's lights~ Won't be implemented because now the controlled train has the red light on and all others have a cyan light on.
* Support more buttons / stick movements
