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

import 'dart:async';
import 'dart:ffi';
import 'dart:io';
import 'dart:isolate';
import 'dart:typed_data';
import 'dart:ui' as ui;
import 'package:ffi/ffi.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutterdoom/engine.dart';
import 'package:flutterdoom/keyboard/ascii_keys.dart';
import 'package:pointer_lock/pointer_lock.dart';

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

class _DoomState extends State<Doom> with WidgetsBindingObserver {
  ui.Image? frame;

  late final FocusNode node;
  late final FocusAttachment nodeAttachment;
  final Map<int, int> phKeyPressed = {};
  bool altLeftActive = false;

  StreamSubscription<PointerLockMoveEvent>? pointerLockSubscription;
  final int mouseBaseSensitivity = 5;
  int mouseButtonPressed = 0;

  final Engine engine = Engine();
  late final DoomModel model;
  ReceivePort? receiveFramePort;

  final int framebufferSize = 64000;
  late final Pointer<UnsignedChar> framebuffer = malloc<UnsignedChar>(framebufferSize);
  late final Uint32List framebuffer32 = Uint32List(framebufferSize);
  late final Pointer<Uint32> palette = malloc<Uint32>(256);

  List<double> aspectRatios = [1.333, 1.6];
  int selectedAspectRatio = 0;

  @override
  void initState() {
    if (kDebugMode) {
      debugPrint("FLUTTERDOOM - Doom.initState()");
    }
    super.initState();

    WidgetsBinding.instance.addObserver(this);

    model = DoomModel();

    node = FocusNode(debugLabel: 'Button');
    node.requestFocus();
    nodeAttachment = node.attach(context, onKeyEvent: _handleKeyPress);

    Engine engine = Engine();

    if (receiveFramePort == null) {
      receiveFramePort = ReceivePort();
      engine.registerDartFramePort(receiveFramePort!.sendPort.nativePort);
    }

    receiveFramePort!.listen((dynamic message) async {
      // Invoked at new frame ready
      for (int i=0; i<framebufferSize; i++) {
        framebuffer32[i] = palette[framebuffer[i]];
      }

      final ui.ImmutableBuffer immutableBuffer = await ui.ImmutableBuffer.fromUint8List(framebuffer32.buffer.asUint8List());

      final ui.Codec codec = await ui.ImageDescriptor.raw(
        immutableBuffer,
        width: 320,
        height: 200,
        rowBytes: null,
        pixelFormat: ui.PixelFormat.rgba8888,
      ).instantiateCodec();

      final ui.FrameInfo frameInfo = await codec.getNextFrame();
          
      final oldFrame = frame;
      frame = frameInfo.image;
      oldFrame?.dispose(); 

      model.refresh();
      
      codec.dispose();
      immutableBuffer.dispose();
    });

    engine.flutterDoomStart(widget.wadPath.toNativeUtf8(), framebuffer, palette);
  }

  @override
  void dispose() {
    var receviPortToClose = receiveFramePort;
    receiveFramePort = null;
    receviPortToClose!.close();
    node.dispose();
    model.dispose();
    WidgetsBinding.instance.removeObserver(this);
    super.dispose();
  }

  @override
  void didChangeAppLifecycleState(AppLifecycleState state) {
    if (
      state == AppLifecycleState.inactive ||
      state == AppLifecycleState.detached ||
      state == AppLifecycleState.hidden ||
      state == AppLifecycleState.paused
    ) {
      if (kDebugMode) {
        debugPrint("FLUTTERDOOM - Focus lost -> sending key-up events to the Doom engine for all pressed keys");
      }
      for (int key in phKeyPressed.keys) {
        engine.dartPostInput(0, key, 0, 0);
      }
      phKeyPressed.clear();
    }
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
      destWidth = destHeight * aspectRatios[selectedAspectRatio];
      if (destWidth > mediaquery.size.width) {
        destWidth = mediaquery.size.width - mediaquery.padding.left - mediaquery.padding.right;
        destHeight = destWidth / aspectRatios[selectedAspectRatio];
      }
    }
    else {
      destWidth = mediaquery.size.width - mediaquery.padding.left - mediaquery.padding.right;
      destHeight = destWidth / aspectRatios[selectedAspectRatio];
    }

    return Listener(
      onPointerMove: Platform.isAndroid || Platform.isIOS ? (e) => _handleMouseMove(e.delta) : null,
      onPointerDown: Platform.isAndroid || Platform.isIOS ? null : _handleMouseClick,
      onPointerUp: Platform.isAndroid || Platform.isIOS ? null : _handleMouseClick,
      child: ListenableBuilder(
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
            return Center(
              child: CustomPaint(
                willChange: true,
                painter: FramebufferPainter(width: destWidth, height: destHeight, frame: frame!),
                size: ui.Size(destWidth, destHeight)
              )
            );
          }
        }
      )
    );
  }

  void _handleMouseMove(Offset delta) {
    engine.dartPostInput(2, mouseButtonPressed, (delta.dx * mouseBaseSensitivity).round(), -(delta.dy * mouseBaseSensitivity).round());
  }

  void _handleMouseClick([PointerEvent? event]) {
    if (pointerLockSubscription != null && event != null) {
      mouseButtonPressed = event.buttons;
      engine.dartPostInput(2, mouseButtonPressed, 0, 0);
      return;
    }

    pointerLockSubscription = pointerLock.createSession(
      windowsMode: PointerLockWindowsMode.capture,
      cursor: PointerLockCursor.hidden
    )
    .listen(
      (event) {
        _handleMouseMove(event.delta);
      },
      onError: (error) {
        if (kDebugMode) {
          debugPrint('FLUTTERDOOM - Pointer lock error: $error');          
        }
      },
    );
  }

  KeyEventResult _handleKeyPress(FocusNode node, KeyEvent event) {
    if (event.synthesized) return KeyEventResult.ignored;

    int asciiCode;

    //print(event.logicalKey.keyLabel);

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

      case "Shift Left":
      case "Shift Right":
        asciiCode = AsciiKeys.keyCodes["KEY_RSHIFT"]!;
        break;

      case "Backspace":
        asciiCode = AsciiKeys.keyCodes["KEY_BACKSPACE"]!;
        break;
      
      case "R":
        if (altLeftActive && event is KeyDownEvent) {
          setState(() {
            selectedAspectRatio = selectedAspectRatio == 0 ? 1 : 0;
          });
        }
        return KeyEventResult.handled;

      case "Q":
        if (altLeftActive) {
          var pointerLockSubscriptionToRemove = pointerLockSubscription;
          pointerLockSubscription = null;
          pointerLockSubscriptionToRemove?.cancel();
        }
        return KeyEventResult.handled;

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
        altLeftActive = event is KeyDownEvent;
        asciiCode = AsciiKeys.keyCodes["KEY_LALT"]!;
        break;

      case "Alt Right":
        if (Platform.isMacOS || Platform.isIOS) {
          asciiCode = AsciiKeys.keyCodes["KEY_RCTRL"]!;
        }
        else {
          asciiCode = AsciiKeys.keyCodes["KEY_RALT"]!;
        }
        break;

      default:
        asciiCode = event.logicalKey.keyId;
    }

    bool keyDownEvent = event is KeyDownEvent || event is KeyRepeatEvent;

    if (keyDownEvent) {
      phKeyPressed[asciiCode] = 1;
      engine.dartPostInput(1, asciiCode, 0, 0);
    }
    else if (phKeyPressed.containsKey(asciiCode)) {
      engine.dartPostInput(0, asciiCode, 0, 0);
      phKeyPressed.remove(asciiCode);
    }
    
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