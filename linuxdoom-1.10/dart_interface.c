/*
 * Copyright (C) 2025 Domenico Muti
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 31 Milk St # 960789 Boston, MA 02196 USA.
 */

#include "dart_interface.h"
#include "dart/dart_api_dl.h"

Dart_Port dart_frame_port = ILLEGAL_PORT;
Dart_Port dart_message_port = ILLEGAL_PORT;

void RegisterDartFramePort(Dart_Port port)
{
    dart_frame_port = port;
}

void RegisterDartGenericPort(Dart_Port port)
{
    dart_message_port = port;
}

void NotifyDartFrameReady()
{
    if (dart_frame_port == ILLEGAL_PORT) {
        return;
    }
    Dart_PostInteger_DL(dart_frame_port, 0);
}

void sendDartMessage(const char* key, const double value)
{
    if (dart_message_port == ILLEGAL_PORT) {
        return;
    }
    
    Dart_CObject key_obj;
    key_obj.type = Dart_CObject_kString;
    key_obj.value.as_string = key;

    Dart_CObject value_obj;
    value_obj.type = Dart_CObject_kDouble;
    value_obj.value.as_double = value; 

    Dart_CObject* array_elements[2];
    array_elements[0] = &key_obj;
    array_elements[1] = &value_obj;

    Dart_CObject array_message;
    array_message.type = Dart_CObject_kArray;
    array_message.value.as_array.length = 2;
    array_message.value.as_array.values = array_elements;

    Dart_PostCObject_DL(dart_message_port, &array_message);
}