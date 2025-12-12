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

import 'dart:ffi';
import 'dart:io';
import 'dart:isolate';
import 'dart:typed_data';
import 'package:ffi/ffi.dart';
import 'package:flutter/material.dart';

class Engine {
  static final Engine _instance = Engine._constructor();

  late final DynamicLibrary dylib;
  late final int Function(Pointer<Void>) dartInitializeApiDL;
  late final void Function(int) registerDartPort;
  late final void Function(Pointer<Utf8>, Pointer<UnsignedChar>, Pointer<Uint32>) flutterDoomStart;
  late final void Function(int, int) dartPostInput;
  late final void Function() flutterDoomQuit;

  final int framebufferSize = 64000;
  late final Pointer<UnsignedChar> framebuffer;
  late final Uint32List framebuffer32;
  late final Pointer<Uint32> palette;

  late final AppLifecycleListener appLifecycleListener;

  factory Engine() {
    return _instance;
  }

  Engine._constructor() {
    if (Platform.isIOS) {
      dylib = DynamicLibrary.process();
    }
    else {
      dylib = DynamicLibrary.open('libdoom.so');
    }

    dartInitializeApiDL = dylib.lookup<NativeFunction<IntPtr Function(Pointer<Void>)>>('Dart_InitializeApiDL').asFunction();
    registerDartPort = dylib.lookup<NativeFunction<Void Function(Int64)>>('RegisterDartPort').asFunction();
    flutterDoomStart = dylib.lookup<NativeFunction<Void Function(Pointer<Utf8>, Pointer<UnsignedChar>, Pointer<Uint32>)>>('FlutterDoomStart').asFunction();
    dartPostInput = dylib.lookup<NativeFunction<Void Function(Int32, Int32)>>('DartPostInput').asFunction();
    flutterDoomQuit = dylib.lookup<NativeFunction<Void Function()>>('I_Quit').asFunction();

    framebuffer = malloc<UnsignedChar>(framebufferSize);
    framebuffer32 = Uint32List(framebufferSize);
    palette = malloc<Uint32>(256);

    // TODO: SISTEMARE I_QUIT, NON DEVE PIù ROMPERSI

    appLifecycleListener = AppLifecycleListener(
      onStateChange: (AppLifecycleState state) {
        if (state == AppLifecycleState.detached) {
          debugPrint('HAURA - App is in detached state. Deallocating Oboe...');
          flutterDoomQuit();
          sleep(Duration(milliseconds: 30));
          dylib.close();
        }
        else if (state == AppLifecycleState.paused) {
          debugPrint('HAURA - App is paused. Pausing Oboe...');

        }
        else if (state == AppLifecycleState.resumed) {
          debugPrint('HAURA - App is resumed. Resuming Oboe...');

        }
      }
    );

    dartInitializeApiDL(NativeApi.initializeApiDLData);
    
    ReceivePort receivePort = ReceivePort();
    registerDartPort(receivePort.sendPort.nativePort);

    receivePort.listen((dynamic message) async {

    });
  }
}