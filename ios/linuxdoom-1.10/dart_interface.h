#ifndef DART_INTERFACE_H
#define DART_INTERFACE_H

#include "dart/dart_api_dl.h"

__attribute__((visibility("default"))) __attribute__((used))
void registerDartPort(Dart_Port port);

__attribute__((visibility("default"))) __attribute__((used))
void NotifyDartFrameReady();

#endif