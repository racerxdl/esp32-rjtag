#!/usr/bin/env python3

import serial, time, struct, os, sys
from commands import *

device = "/dev/ttyUSB0"
baudrate = 921600

if len(sys.argv) < 6:
  print("Usage: setwificonfig.py /dev/ttyUSB0 WIFI_SSID WIFI_PASSWORD OTA_PASSWORD [hostname]")
  exit(1)

usbport = sys.argv[1]
wifissid = sys.argv[2]
wifipass = sys.argv[3]
otapass = sys.argv[4]
hostname = None

if len(sys.argv) >= 6:
  hostname = sys.argv[5]

print("[HOST] Connecting to %s at %d" %(usbport, baudrate))
ser = serial.Serial(usbport, baudrate, timeout=1, xonxoff=0, rtscts=0)  # open serial port

print("[HOST] Resetting")
ser.setDTR(False)
time.sleep(0.1)
ser.flushInput()
ser.setDTR(True)

offset = 0


print("[HOST] Waiting ESP32 to be ready")
while True:
  try:
    line = ser.readline().decode("utf-8", errors="ignore")
    if "[RDY]" in line:
      if hostname != None:
        SetHostname(ser, hostname)
      SetWifiSSID(ser, wifissid)
      SetWifiPassword(ser, wifipass)
      SetOTAPassword(ser, otapass)
      Reboot(ser)
      break
  except UnicodeDecodeError as e:
    print(e)
    continue
  except Exception as e:
    raise e
    break

ser.close()             # close port
