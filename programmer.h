#pragma once

#define PIN_TDI 33
#define PIN_TDO 32
#define PIN_TCK 27
#define PIN_TMS 26
#define PIN_LED 2

#define CMD_DATA          'd'
#define CMD_SET_HOSTNAME  'h'
#define CMD_START_SVF     'i'
#define CMD_SET_WIFI_PASS 'l'
#define CMD_SET_OTA_PASS  'o'
#define CMD_PASSTHROUGH   'p'
#define CMD_QUERY         'q'
#define CMD_REBOOT        'r'
#define CMD_STOP          's'
#define CMD_SET_WIFI_SSID 'u'
#define CMD_START_XSVF    'x'
#define CMD_GET_STATE     'g'

void ProgrammerLoop();
void ProgrammerInit();
