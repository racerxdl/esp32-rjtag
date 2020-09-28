#pragma once
#include "events.h"

template<typename... Args> void Info(const char * fmt, Args... args) {
  websocket.printfAll(fmt, args...);
  Serial.printf(fmt, args...);
}

template<typename... Args> void Debug(const char * fmt, Args... args) {
  // websocket.printfAll(fmt, args...);
  // Serial.printf(fmt, args...);
}

template<typename... Args> void Error(const char * fmt, Args... args) {
  websocket.printfAll(fmt, args...);
  Serial.printf(fmt, args...);
}

