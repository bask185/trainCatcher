#!/usr/bin/env python
import os
os.system("python src/build.py")
print("UPLOADING")
#os.system("arduino-cli upload -b attiny:avr:attinyX5 -p COM4 -i ./build/attiny.avr.ATtinyX5/trainCatcher.ino.hex")
#os.system("arduino-cli upload -b ATTinyCore:avr:attinyx5:chip=45,clock=1internal --programmer arduinoasisp -p COM4 -i ./build/ATTinyCore.avr.attinyx5/trainCatcher.ino.hex")
os.system("arduino-cli upload -b arduino:avr:nano -p COM4 -i ./build/arduino.avr.nano/trainCatcher.ino.hex")
exit