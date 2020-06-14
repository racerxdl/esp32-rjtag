#!/usr/bin/env python3

import serial, time, struct, os, sys

device = "/dev/ttyUSB0"
baudrate = 921600

if len(sys.argv) < 3:
  print("Usage: upload.py /dev/ttyUSB0 file.svf")
  exit(1)

usbport = sys.argv[1]
filename = sys.argv[2]

f = open(filename, "rb")

filelen = os.fstat(f.fileno()).st_size

print("[HOST] Connecting to %s at %d" %(usbport, baudrate))
ser = serial.Serial(usbport, baudrate, timeout=1, xonxoff=0, rtscts=0)  # open serial port

print("[HOST] Resetting")
ser.setDTR(False)
time.sleep(0.1)
ser.flushInput()
ser.setDTR(True)

CMD_START_SVF   = b'i'
CMD_START_XSVF  = b'x'
CMD_DATA        = b'd'
CMD_STOP        = b's'
CMD_QUERY       = b'q'
CMD_PASSTHROUGH = b'p'

offset = 0

def processCtrl(line):
  global offset
  global ser

  line = line[7:]
  ctrl = line[0]
  data = line[1:]

  if ctrl == "R":
    length = int(data)
    # print("Requested %d bytes" % length)
    buff = f.read(length)
    # print("Read %d bytes" % len(buff))
    if len(buff) == 0:
      ser.write(struct.pack(">BI", CMD_STOP[0], 0))
      # print("\nFinished")
    else:
      l = struct.pack(">BI", CMD_DATA[0], len(buff))
      ser.write(l)
      ser.write(buff)
    if offset > 0: # Skip progressing first block
      offset += len(buff)
      sys.stdout.write("\r[HOST] Progress: %8d / %8d - %0.2f%%" % (offset, filelen, 100 * offset / filelen))
    else:
      offset += len(buff)
  elif ctrl == "A":
    # print("Received ACK")
    pass
  elif ctrl == "M":
    # print("Control Message: %s" % data)
    pass
  if offset == filelen:
    sys.stdout.write("\n")

qb = CMD_QUERY + b'\x00\x00\x00\x00'
qs = CMD_START_SVF + b'\x00\x00\x00\x00'

qp = CMD_PASSTHROUGH + struct.pack(">I", 115200)

print("[HOST] Waiting ESP32 to be ready")
while True:
  try:
    line = ser.readline().decode("utf-8", errors="ignore")
    if "[RDY]" in line:
      ser.write(qb)         # write a string
    elif "[QUERY]" in line:
      print(line.strip())
      ser.write(qs)
    elif "[JTAG]" in line:
      print(line.strip())
      if "Programming finished" in line:
        print("[HOST] Enabling passthrough")
        ser.write(qp)
    elif "[CTRL]" in line:
      processCtrl(line)
    elif "[PASS] READY" in line:
      print("[PASS] READY")
    else:
      if len(line) > 0:
        print(line.strip())
  except UnicodeDecodeError as e:
    print(e)
    continue
  except Exception as e:
    raise e
    break

ser.close()             # close port
f.close()