#include "storage.h"
#include <EEPROM.h>

#define SSID_LENGTH 64
#define PASSWORD_LENGTH 64
#define HOSTNAME_LENGTH 32

struct Config {
  // WIFI
  char SSID[SSID_LENGTH];
  char WifiPassword[PASSWORD_LENGTH];
  char Hostname[HOSTNAME_LENGTH];
  char OTAPassword[PASSWORD_LENGTH];
} currentConfig;

const size_t ConfigLength = sizeof(currentConfig);

void ReadConfig() {
  char *c = (char *)(&currentConfig);
  for (int i=0; i<ConfigLength;i++) {
    c[i] = EEPROM.read(i);
  }

  // Pad all strings to be null terminated
  currentConfig.SSID[SSID_LENGTH-1] = 0x00;
  currentConfig.WifiPassword[PASSWORD_LENGTH-1] = 0x00;
  currentConfig.OTAPassword[PASSWORD_LENGTH-1] = 0x00;
  currentConfig.Hostname[HOSTNAME_LENGTH-1] = 0x00;
}

void SaveConfig() {
  currentConfig.SSID[SSID_LENGTH-1] = 0x00;
  currentConfig.WifiPassword[PASSWORD_LENGTH-1] = 0x00;
  currentConfig.OTAPassword[PASSWORD_LENGTH-1] = 0x00;
  currentConfig.Hostname[HOSTNAME_LENGTH-1] = 0x00;

  char *c = (char *)(&currentConfig);
  for (int i=0; i<ConfigLength;i++) {
    EEPROM.write(i, c[i]);
  }
  EEPROM.commit();
}

String GetWifiSSID() {
  return String(currentConfig.SSID);
}
String GetWifiPassword() {
  return String(currentConfig.WifiPassword);
}
String GetOTAPassword() {
  return String(currentConfig.OTAPassword);
}
String GetHostname() {
  return String(currentConfig.Hostname);
}

void SaveWifiSSID(String ssid) {
    int maxLen = SSID_LENGTH-1;
    if (ssid.length() < maxLen) {
      maxLen = ssid.length();
    }
    for (int i = 0; i < SSID_LENGTH; i++) {
      if (i < maxLen) {
        currentConfig.SSID[i] = ssid[i];
      } else {
        currentConfig.SSID[i] = 0x00;
      }
    }
    SaveConfig();
}

void SaveWifiPassword(String pass) {
    int maxLen = PASSWORD_LENGTH-1;
    if (pass.length() < maxLen) {
      maxLen = pass.length();
    }
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
      if (i < maxLen) {
        currentConfig.WifiPassword[i] = pass[i];
      } else {
        currentConfig.WifiPassword[i] = 0x00;
      }
    }
    SaveConfig();
}


void SaveOTAPassword(String pass) {
    int maxLen = PASSWORD_LENGTH-1;
    if (pass.length() < maxLen) {
      maxLen = pass.length();
    }
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
      if (i < maxLen) {
        currentConfig.OTAPassword[i] = pass[i];
      } else {
        currentConfig.OTAPassword[i] = 0x00;
      }
    }
    SaveConfig();
}

void SaveHostname(String hostname) {
    int maxLen = HOSTNAME_LENGTH-1;
    if (hostname.length() < maxLen) {
      maxLen = hostname.length();
    }
    for (int i = 0; i < HOSTNAME_LENGTH; i++) {
      if (i < maxLen) {
        currentConfig.Hostname[i] = hostname[i];
      } else {
        currentConfig.Hostname[i] = 0x00;
      }
    }
    SaveConfig();
}

void InitStorage() {
  EEPROM.begin(ConfigLength);
  ReadConfig();

  if (GetHostname() == "") {
    Serial.println("[INFO] No hostname set. Setting to esp32-rjtag");
    SaveHostname("esp32-rjtag");
  }

  if (GetOTAPassword() == "") {
    Serial.println("[INFO] No OTA Password set. Setting to 1234567890");
    SaveOTAPassword("1234567890");
  }
}
