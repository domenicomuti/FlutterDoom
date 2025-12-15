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
import 'dart:isolate';
import 'dart:ui' as ui;
import 'package:ffi/ffi.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutterdoom/engine.dart';
import 'package:flutterdoom/keyboard/ascii_keys.dart';

class DoomModel extends ChangeNotifier {
  void refresh() {
    notifyListeners();
  }
}

class Doom extends StatefulWidget {
  final String wadPath;

  const Doom({super.key, required this.wadPath});
  
  @override
  State<Doom> createState() => _DoomState();
}

class _DoomState extends State<Doom> {
  ui.Image? frame;

  late final FocusNode node;
  late final FocusAttachment nodeAttachment;
  final Engine engine = Engine();
  late final DoomModel model;
  late ReceivePort receiveFramePort;

  @override
  void initState() {
    if (kDebugMode) {
      debugPrint("FLUTTERDOOM - Doom.initState()");
    }
    super.initState();

    model = DoomModel();

    node = FocusNode(debugLabel: 'Button');
    node.requestFocus();
    nodeAttachment = node.attach(context, onKeyEvent: _handleKeyPress);

    Engine engine = Engine();
    
    receiveFramePort = ReceivePort();
    engine.registerDartFramePort(receiveFramePort.sendPort.nativePort);

    receiveFramePort.listen((dynamic message) async {
      // Invoked at new frame ready
      for (int i=0; i<engine.framebufferSize; i++) {
        engine.framebuffer32[i] = engine.palette[engine.framebuffer[i]];
      }

      ui.ImmutableBuffer immutableBuffer = await ui.ImmutableBuffer.fromUint8List(engine.framebuffer32.buffer.asUint8List());

      ui.Codec codec = await ui.ImageDescriptor.raw(
        immutableBuffer,
        width: 320,
        height: 200,
        rowBytes: null,
        pixelFormat: ui.PixelFormat.rgba8888, 
      ).instantiateCodec();

      ui.FrameInfo frameInfo = await codec.getNextFrame();
      
      frame = frameInfo.image;

      model.refresh();
    });

    engine.flutterDoomStart(widget.wadPath.toNativeUtf8(), engine.framebuffer, engine.palette);
  }

  @override
  void dispose() {
    receiveFramePort.close();
    node.dispose();
    model.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    if (kDebugMode) {
      print("FLUTTERDOOM - Doom.build()");
    }

    nodeAttachment.reparent();

    var mediaquery = MediaQuery.of(context);

    double destWidth;
    double destHeight;
    
    if (mediaquery.size.width >= mediaquery.size.height) {
      destHeight = mediaquery.size.height - mediaquery.padding.top - mediaquery.padding.bottom;
      destWidth = destHeight * 1.6;
      if (destWidth > mediaquery.size.width) {
        destWidth = mediaquery.size.width - mediaquery.padding.left - mediaquery.padding.right;
        destHeight = destWidth / 1.6;
      }
    }
    else {
      destWidth = mediaquery.size.width - mediaquery.padding.left - mediaquery.padding.right;
      destHeight = destWidth / 1.6;
    }

    return ListenableBuilder(
      listenable: model,
      builder: (context, child) {
        if (frame == null) {
          return SizedBox(
            width: destWidth,
            height: destHeight,
            child: Center(child: Text("Doom is starting...", style: TextStyle(color: Colors.grey))),
          );
        }
        else {
          return Center(child: CustomPaint(
            willChange: true,
            painter: FramebufferPainter(width: destWidth, height: destHeight, frame: frame!),
            size: ui.Size(destWidth, destHeight)
          ));
        }
      }
    );
  }

  KeyEventResult _handleKeyPress(FocusNode node, KeyEvent event) {
    int asciiCode;

    switch (event.logicalKey.keyLabel) {
      case "Arrow Right":
        asciiCode = AsciiKeys.keyCodes["KEY_RIGHTARROW"]!;
        break;

      case "Arrow Left":
        asciiCode = AsciiKeys.keyCodes["KEY_LEFTARROW"]!;
        break;

      case "Arrow Up":
        asciiCode = AsciiKeys.keyCodes["KEY_UPARROW"]!;
        break;

      case "Arrow Down":
        asciiCode = AsciiKeys.keyCodes["KEY_DOWNARROW"]!;
        break;

      case "Escape":
      case "Go Back":
        asciiCode = AsciiKeys.keyCodes["KEY_ESCAPE"]!;
        break;

      case "Enter":
      case "Numpad Enter":
        asciiCode = AsciiKeys.keyCodes["KEY_ENTER"]!;
        break;

      case "Tab":
        asciiCode = AsciiKeys.keyCodes["KEY_TAB"]!;
        break;

      case "Control Left":
      case "Control Right":
        asciiCode = AsciiKeys.keyCodes["KEY_RCTRL"]!;
        break;

      case "Shift Right":
      case "Shift Left":
        asciiCode = AsciiKeys.keyCodes["KEY_RSHIFT"]!;
        break;

      case "Backspace":
        asciiCode = AsciiKeys.keyCodes["KEY_BACKSPACE"]!;
        break;

      case "F1":
        asciiCode = AsciiKeys.keyCodes["KEY_F1"]!;
        break;

      case "F2":
        asciiCode = AsciiKeys.keyCodes["KEY_F2"]!;
        break;

      case "F3":
        asciiCode = AsciiKeys.keyCodes["KEY_F3"]!;
        break;

      case "F4":
        asciiCode = AsciiKeys.keyCodes["KEY_F4"]!;
        break;

      case "F5":
        asciiCode = AsciiKeys.keyCodes["KEY_F5"]!;
        break;

      case "F6":
        asciiCode = AsciiKeys.keyCodes["KEY_F6"]!;
        break;

      case "F7":
        asciiCode = AsciiKeys.keyCodes["KEY_F7"]!;
        break;

      case "F8":
        asciiCode = AsciiKeys.keyCodes["KEY_F8"]!;
        break;

      case "F9":
        asciiCode = AsciiKeys.keyCodes["KEY_F9"]!;
        break;

      case "F10":
        asciiCode = AsciiKeys.keyCodes["KEY_F10"]!;
        break;

      case "F11":
        asciiCode = AsciiKeys.keyCodes["KEY_F11"]!;
        break;

      case "F12":
        asciiCode = AsciiKeys.keyCodes["KEY_F12"]!;
        break;

      case "Pause":
        asciiCode = AsciiKeys.keyCodes["KEY_PAUSE"]!;
        break;

      case "Alt Left":
        asciiCode = AsciiKeys.keyCodes["KEY_LALT"]!;
        break;

      case "Alt Right":
        asciiCode = AsciiKeys.keyCodes["KEY_RALT"]!;
        break;

      case "Z":
        asciiCode = AsciiKeys.keyCodes[","]!;
        break;

      case "X":
        asciiCode = AsciiKeys.keyCodes["."]!;
        break;

      default:
        asciiCode = event.logicalKey.keyId;
    }

    //print(event);

    engine.dartPostInput(asciiCode, event is KeyDownEvent || event is KeyRepeatEvent ? 1 : 0);
    return KeyEventResult.handled;
  }
}

class FramebufferPainter extends CustomPainter {
  double width;
  double height;
  ui.Image frame;

  FramebufferPainter({required this.width, required this.height, required this.frame});

  @override
  void paint(ui.Canvas canvas, ui.Size size) {
    Rect src = Rect.fromLTWH(0, 0, frame.width.toDouble(), frame.height.toDouble());
    Rect dst = Rect.fromLTWH(0, 0, width, height);
    
    canvas.drawImageRect(frame, src, dst, Paint());
  }
  
  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => true;
}