#pragma once

#include "Arduino.h"

String GetWifiSSID();
String GetWifiPassword();
String GetHostname();
String GetOTAPassword();

void SaveWifiSSID(String ssid);
void SaveWifiPassword(String pass);
void SaveHostname(String hostname);
void SaveOTAPassword(String password);

void InitStorage();