# grbl_controller_stm32
Grbl controller running on stm32F103

This project allows to control a CNC running GRBL without having to use a pc.

This is an alternative to Marlin or Repetier foc CNC.

It uses a blue pill board (with a STM32F103 chip) and a 128 X 64 Reprap display (with a rotary switch and SD card reader).

This applications allows to:
- select a file on the SD card with Gcode and to send it to GRBL
- pause/resume/cancel sending the Gcode
- send predefined GRBL commands
- unlock alarm
- ask for homing the CNC
- move X,Y, Z axis by 0.01, 0.1, 1, 10 mm steps
- set X, Y, Z Work position to 0 (based on the current position)
- forward GRBL commands from the PC (so you can still control your CNC using your pc with e.g. Universal Gcode sender).
  This uses the USB interface that exist on the blue pill.

This application displays some GRBL informations like
- the GRBL status (Idle, Run, Alarm,...)
- the work position (Wpos) and the machine position (Mpos)
- the last error and alert message.

Optionnally you can connect a Nunchuck in order to move the X, Y, Z, axis with the joystick 
- press the C button to move X/Y axis,
- press the Z button to move Z axis

The pins being used are defined in the config.h file
Current pins have been selected to make an easy connection with the display module
  Pins are in the same sequence as the wires from the cable to the display. 

Important note: in order to compile this project, you have to add the library SdFat to your arduino IDE.
You must edit the file SdFatConfig.h (normally in folder documents/arduino/librairies/sdFat/src).
In line 78 you must set the parameter to "0"; so you must have
#define USE_STANDARD_SPI_LIBRARY 0
