#!/usr/bin/env python3

import serial, time, struct, os, sys

from commands import *

import asyncio
import websockets

if len(sys.argv) < 3:
  print("Usage: upload.py IP file.svf")
  exit(1)

ip = sys.argv[1]
filename = sys.argv[2]

f = open(filename, "rb")

filelen = os.fstat(f.fileno()).st_size

JTAGRESULT = "JTAG Result"

async def queryChip(websocket):
  '''
    Queries the CHIP ID
  '''
  data = bytearray(CMD_QUERY + "\x00\x00", encoding="utf8")
  # print(f"> [CMD_QUERY]")
  await websocket.send(data)
  data = await websocket.recv()
  expectedPath = "[INFO] Chip ID:"
  if len(data) > len(expectedPath):
    return data[len(expectedPath):].strip()
  return None

async def startXSVF(websocket):
  '''
    Start XSVF Transfer
  '''
  data = bytearray(CMD_START_XSVF + "\x00\x00", encoding="utf8")
  # print(f"> [CMD_START_XSVF]")
  await websocket.send(data)

async def startSVF(websocket):
  '''
    Start SVF Transfer
  '''
  data = bytearray(CMD_START_SVF + "\x00\x00", encoding="utf8")
  # print(f"> [CMD_START_SVF]")
  await websocket.send(data)

async def putData(websocket, data):
  '''
    Puts DATA into CMD_DATA queue
  '''
  senddata = struct.pack(">BH", ord(CMD_DATA), 0)
  senddata += bytearray(data)
  #print(f"> [CMD_DATA]")
  await websocket.send(senddata)

def GetRequestLength(data):
  prefix = "[INFO] R"
  if len(data) < len(prefix):
    return 0

  data = data[len(prefix):]
  return int(data)

async def WaitRequestLength(websocket):
  prefix = "[INFO] R"
  resp = ""
  while not prefix in str(resp):
    resp = await websocket.recv()
    if JTAGRESULT in str(resp): # Early result
      raise Exception(resp)
    #print(f"< %s" % resp)
  return GetRequestLength(resp)

async def hello():
    uri = "ws://%s/ws" % ip
    print(f"> [INFO] Connecting to %s" % uri)
    async with websockets.connect(uri) as websocket:
      print(f"> [INFO] Connected to %s" %uri)
      greeting = await websocket.recv()
      print(f"< {greeting}")

      chipId = await queryChip(websocket)
      print(f"< [INFO] Chip ID: %s" % chipId)

      print(f"> [INFO] Starting SVF")
      await startSVF(websocket)

      print(f"> [INFO] Sending file %s" % filename)
      readBytes = 0
      start = time.time()
      while readBytes < filelen:
        reqLen = await WaitRequestLength(websocket)
        #print(f"Requested chunk length: %d" % reqLen)
        data = f.read(reqLen)
        readBytes += len(data)
        sys.stdout.write("\r> [INFO] Progress: %8d / %8d - %0.2f%%" % (readBytes, filelen, 100 * readBytes / filelen))
        await putData(websocket, data)
      delta = time.time() - start
      print("\n> [INFO] Took %f seconds to upload" % delta)
    print("> [INFO] Closing...")
    return None

asyncio.get_event_loop().run_until_complete(hello())