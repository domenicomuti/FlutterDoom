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

#ifndef DART_INTERFACE_H
#define DART_INTERFACE_H

#include "dart/dart_api_dl.h"
#include "doomdef.h"

DART_VISIBILITY
void RegisterDartFramePort(Dart_Port port);

DART_VISIBILITY
void RegisterDartGenericPort(Dart_Port port);

DART_VISIBILITY
void NotifyDartFrameReady();

DART_VISIBILITY
void sendDartMessage(const char* key, const double value);

#endif