#include "dart_interface.h"

Dart_Port dart_receive_port = ILLEGAL_PORT;

void RegisterDartPort(Dart_Port port) {
    dart_receive_port = port;
}

void NotifyDartFrameReady() {
    if (dart_receive_port == ILLEGAL_PORT) {
        return;
    }
    Dart_PostInteger_DL(dart_receive_port, 0);
}