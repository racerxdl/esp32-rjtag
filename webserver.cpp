#include "webserver.h"

#include <FS.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <AsyncTCP.h>

#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>

#include "events.h"

#include "program.h"
#include "programmer.h"
#include "netlog.h"

AsyncWebServer server(80);

#define STATE_IDLE 1
#define STATE_WAITING_PACKET 2
#define STATE_BLOCK_READY 3

#define MULTI_FRAME_SIZE (BUFFER_SIZE + 80)

struct ClientState {
  uint8_t codeBuffer[BUFFER_SIZE];
  size_t length;
  uint8_t currentState;
  AsyncWebSocketClient *currentClient;
  int dataType;

  // Multi-frame suport
  uint8_t multiFrameBuffer[MULTI_FRAME_SIZE];
  size_t multiFrameBufferPos;
  uint8_t multiFrameValid;

  // Running JTAG Task
  uint8_t programming;
} cs;


TaskHandle_t task_jtag;

uint8_t isWifiProgramming() {
  return cs.programming;
}

void vJTAG(void *pvParameters){
  cs.programming = 1;
  Info("[INFO] JTAG Task Started\r\n");
  int result = jtag_program(cs.dataType, MODE_WIFI);
  Info("[INFO] JTAG Result: %d\r\n", result);
  cs.programming = 0;
  vTaskDelete(NULL);
}

int fetch_next_block_wifi(uint8_t *buffer, int length) {
  while (cs.currentState != STATE_BLOCK_READY) {
    if (cs.currentState == STATE_IDLE) { // Close task
      Debug("[DEBUG] JTAG Stop Block fetching\r\n");
      return 0;
    }
    delay(1); // Wait 1ms, that also yields the processor to other tasks
  }

  if (cs.length > length) {
    Error("[ERROR] Data bigger than expected buffer size\r\n");
    return -1;
  }

  int readBytes = 0;

  while (readBytes < cs.length) {
    buffer[readBytes] = cs.codeBuffer[readBytes];
    readBytes++;
  }

  cs.currentState = STATE_WAITING_PACKET;

  if (cs.currentClient != NULL) {
    cs.currentClient->printf("[CTRL] R%d", BUFFER_SIZE);
  }
  Debug("[INFO] Received %d bytes of data...\r\n", readBytes);

  return readBytes;
  // return -1;
}

void onText(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  data[len] = 0x00; // Ensure null-terminated
  Debug("[WS][%u]: %s\r\n", client->id(), data);
  client->text("[ERROR] Thanks, but no text here...");
}

void notSupported(AsyncWebSocketClient *client, uint8_t cmd) {
  Error("[ERROR] Command '%c' not supported...\r\n", cmd);
}

void notImplemented(AsyncWebSocketClient *client, uint8_t cmd) {
  Error("[ERROR] Command '%c' not implemented...\r\n", cmd);
}

void cmdQuery(AsyncWebSocketClient *client) {
  uint32_t chipId = jtag_chip_id();
  Info("[INFO] Chip ID: %08x\r\n", chipId);
}

void cmdStop(AsyncWebSocketClient *client) {
  Info("[INFO] Stopping JTAG\r\n");
  cs.currentState = STATE_IDLE;
}

void cmdReboot(AsyncWebSocketClient *client) {
  Info("[INFO] Received reboot\r\n");
  ESP.restart();
}

void cmdStart(AsyncWebSocketClient *client, int dataType) {
  Info("[INFO] Starting JTAG\r\n");
  cs.currentState = STATE_WAITING_PACKET;
  cs.currentClient = client;
  cs.dataType = dataType;
  xTaskCreatePinnedToCore(vJTAG, "vJTAG", 10000, NULL, tskIDLE_PRIORITY + 1, &task_jtag, 1);
  client->printf("[CTRL] R%d", BUFFER_SIZE);
}

void cmdData(AsyncWebSocketClient *client, uint8_t *data, size_t len) {
  if (len > BUFFER_SIZE) {
    Error("[ERROR] Max buffer length %d but got %d bytes in data packet.\r\n", BUFFER_SIZE, len);
    cs.currentState = STATE_IDLE;
    return;
  }

  cs.length = len;
  memcpy(cs.codeBuffer, data, len);
  cs.currentState = STATE_BLOCK_READY;
}

void onBinary(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  uint8_t cmd = data[0];
  Debug("[WS][%u]: Received command '%c'\r\n", client->id(), cmd);

  switch (cmd) {
    case CMD_DATA:
      return cmdData(client, &data[3], len-3);
    case CMD_SET_HOSTNAME:
      return notImplemented(client, cmd);
    case CMD_START_SVF:
      return cmdStart(client, DATA_TYPE_SVF);
    case CMD_SET_WIFI_PASS:
    case CMD_SET_OTA_PASS:
    case CMD_PASSTHROUGH:
      return notImplemented(client, cmd);
    case CMD_QUERY: return cmdQuery(client);
    case CMD_REBOOT: return cmdReboot(client);
    case CMD_STOP:
      return cmdStop(client);
    case CMD_SET_WIFI_SSID:
      return notImplemented(client, cmd);
    case CMD_START_XSVF:
      return cmdStart(client, DATA_TYPE_XSVF);
    case CMD_GET_STATE:
    default:
      return notSupported(client, cmd);
  }
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    Debug("[WS][%u] connect\r\n", client->id());
    client->printf("[INFO] Hello Client %u :)", client->id());
    // client->ping();
  } else if(type == WS_EVT_DISCONNECT){
    cs.currentClient = NULL;
    Debug("[WS][%u] disconnect\r\n", client->id());
  } else if(type == WS_EVT_ERROR){
    Error("[WS][%u] error(%u): %s\r\n", client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    Debug("[WS][%u] pong[%u]: %s\r\n", client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if(info->final && info->index == 0 && info->len == len) {
      if (info->opcode == WS_TEXT) {
        onText(server, client, type, arg, data, len);
      } else {
        onBinary(server, client, type, arg, data, len);
      }
    } else {
      if (info->index == 0) {
        Debug("[WS][%u] Frame start %d\r\n", client->id(), info->len);
        cs.multiFrameBufferPos = 0;
        cs.multiFrameValid = 1;
      }

      if (!cs.multiFrameValid) {
        Error("[ERROR] Invalid piece of data\r\n");
        return;
      }

      if (info->index + len > MULTI_FRAME_SIZE) {
        Error("[ERROR] Current frame exceeds max storage capacity of %d.\r\n", MULTI_FRAME_SIZE);
        cs.multiFrameValid = 0;
        return;
      }

      memcpy(&cs.multiFrameBuffer[info->index], data, len);
      cs.multiFrameBufferPos = info->index + len;
      Debug("[WS][%u] Received %d bytes.\r\n", client->id(), cs.multiFrameBufferPos);

      if(cs.multiFrameBufferPos == info->len){
        Debug("[WS][%u] Frame end. Received %d bytes.\r\n", client->id(), cs.multiFrameBufferPos);
        if (info->opcode == WS_TEXT) {
          onText(server, client, type, arg, cs.multiFrameBuffer, cs.multiFrameBufferPos);
        } else {
          onBinary(server, client, type, arg, cs.multiFrameBuffer, cs.multiFrameBufferPos);
        }
        cs.multiFrameValid = 0;
        cs.multiFrameBufferPos = 0;
      }
    }
  }
}


void InitWebServer() {
  cs.currentState = STATE_IDLE;
  cs.programming = 0;

  MDNS.addService("http","tcp",80);
  SPIFFS.begin();
  websocket.onEvent(onWsEvent);
  server.addHandler(&websocket);

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");

  server.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index)
      Serial.printf("[WEB] UploadStart: %s\n", filename.c_str());
    Serial.printf("%s", (const char*)data);
    if(final)
      Serial.printf("[WEB] UploadEnd: %s (%u)\n", filename.c_str(), index+len);
  });

  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(!index)
      Serial.printf("[WEB] BodyStart: %u\n", total);
    Serial.printf("%s", (const char*)data);
    if(index + len == total)
      Serial.printf("[WEB] BodyEnd: %u\n", total);
  });

  server.begin();
}


void WebServerLoop() {
  websocket.cleanupClients();
}