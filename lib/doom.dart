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
import 'dart:typed_data';
import 'dart:ui' as ui;
import 'package:ffi/ffi.dart';
import 'package:flutter/material.dart';
import 'package:flutterdoom/engine.dart';

class Doom extends StatefulWidget {
  final String wadPath;

  const Doom({super.key, required this.wadPath});
  
  @override
  State<Doom> createState() => _DoomState();
}

class _DoomState extends State<Doom> {
  final int framebufferSize = 64000;
  late final Pointer<UnsignedChar> framebuffer;
  late final Uint32List framebuffer32;
  late final Pointer<Uint32> palette;

  ui.Image? frame;

  @override
  void initState() {
    super.initState();

    framebuffer = malloc<UnsignedChar>(framebufferSize);
    framebuffer32 = Uint32List(framebufferSize);
    palette = malloc<Uint32>(256);

    Engine engine = Engine();
    
    ReceivePort receiveFramePort = ReceivePort();
    engine.registerDartFramePort(receiveFramePort.sendPort.nativePort);

    receiveFramePort.listen((dynamic message) async {
      // Invoked at new frame ready
      for (int i=0; i<framebufferSize; i++) {
        framebuffer32[i] = palette[framebuffer[i]];
      }

      ui.ImmutableBuffer immutableBuffer = await ui.ImmutableBuffer.fromUint8List(framebuffer32.buffer.asUint8List());

      final ui.Codec codec = await ui.ImageDescriptor.raw(
        immutableBuffer,
        width: 320,
        height: 200,
        rowBytes: null,
        pixelFormat: ui.PixelFormat.rgba8888, 
      ).instantiateCodec();

      final ui.FrameInfo frameInfo = await codec.getNextFrame();
      
      setState(() {
        frame = frameInfo.image;
      });
    });

    engine.flutterDoomStart(widget.wadPath.toNativeUtf8(), framebuffer, palette);
  }

  @override
  Widget build(BuildContext context) {
    var mediaquery = MediaQuery.of(context);
    double destWidth = mediaquery.size.width - mediaquery.padding.left - mediaquery.padding.right;
    double destHeight = destWidth / 1.6;

    if (frame == null) {
      return SizedBox(
        width: destWidth,
        height: destHeight,
        child: Center(child: Text("Doom is starting...")),
      );
    }
    else {
      return CustomPaint(
        painter: FramebufferPainter(width: destWidth, height: destHeight, frame: frame!),
        size: ui.Size(destWidth, destHeight)
      );
    }
  }
}

class FramebufferPainter extends CustomPainter {
  double width;
  double height;
  ui.Image frame;

  FramebufferPainter({required this.width, required this.height, required this.frame});

  @override
  void paint(ui.Canvas canvas, ui.Size size) {
    final Rect src = Rect.fromLTWH(0, 0, frame.width.toDouble(), frame.height.toDouble());
    final Rect dst = Rect.fromLTWH(0, 0, width, height);
    
    canvas.drawImageRect(frame, src, dst, Paint());
  }
  
  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => true;
}