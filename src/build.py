#!/usr/bin/env python
import os
print('ASSEMBLING IO FILES')
os.system("updateIO.py")
print('ADDING TIME STAMP')
os.system("addDate.py")
print('BUILDING PROJECT')
#os.system('arduino-cli compile -b attiny:avr:ATtinyX5 -e')
os.system('arduino-cli compile --warnings more -b arduino:avr:nano:cpu=atmega328old -e')
exit