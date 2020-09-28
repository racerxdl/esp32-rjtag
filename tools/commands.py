#!/usr/bin/env python3

import struct

CMD_DATA          = 'd'
CMD_SET_HOSTNAME  = 'h'
CMD_START_SVF     = 'i'
CMD_SET_WIFI_PASS = 'l'
CMD_SET_OTA_PASS  = 'o'
CMD_PASSTHROUGH   = 'p'
CMD_QUERY         = 'q'
CMD_REBOOT        = 'r'
CMD_STOP          = 's'
CMD_SET_WIFI_SSID = 'u'
CMD_START_XSVF    = 'x'


def __sendStringCMD(ser, cmd, data):
  l = len(data)
  packet = struct.pack(">BI", ord(cmd), l) + bytearray(data, encoding="utf8")
  ser.write(packet)

def SetHostname(ser, name):
  '''
    Sets the ESP32-RJTAG Hostname
  '''
  __sendStringCMD(ser, CMD_SET_HOSTNAME, name)
  print(ser.readline().decode("utf-8", errors="ignore").strip())

def SetWifiSSID(ser, ssid):
  '''
    Sets the ESP32-RJTAG Wifi SSID
  '''
  __sendStringCMD(ser, CMD_SET_WIFI_SSID, ssid)
  print(ser.readline().decode("utf-8", errors="ignore").strip())


def SetWifiPassword(ser, password):
  '''
    Sets the ESP32-RJTAG Wifi Password
  '''
  __sendStringCMD(ser, CMD_SET_WIFI_PASS, password)
  print(ser.readline().decode("utf-8", errors="ignore").strip())


def SetOTAPassword(ser, password):
  '''
    Sets the ESP32-RJTAG Over-The-Air Update Password
  '''
  __sendStringCMD(ser, CMD_SET_OTA_PASS, password)
  print(ser.readline().decode("utf-8", errors="ignore").strip())


def Reboot(ser):
  __sendStringCMD(ser, CMD_REBOOT, "")
  print(ser.readline().decode("utf-8", errors="ignore").strip())

