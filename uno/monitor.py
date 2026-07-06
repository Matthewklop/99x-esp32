import sys
import time

import serial

s = serial.Serial("/dev/ttyACM0", 115200, timeout=1)
s.setDTR(0)
time.sleep(0.1)
s.setRTS(0)
time.sleep(0.5)
s.setDTR(1)
time.sleep(0.1)
s.setDTR(0)
time.sleep(2)
try:
    while True:
        d = s.read(4096)
        if d:
            sys.stdout.buffer.write(d)
            sys.stdout.flush()
except KeyboardInterrupt:
    pass
