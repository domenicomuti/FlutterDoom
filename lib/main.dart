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

import 'dart:io';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutterdoom/doom.dart';
import 'package:flutterdoom/engine.dart';
import 'package:flutterdoom/keyboard/portrait_keyboard.dart';
import 'package:path_provider/path_provider.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();

  Engine();   // Initialize the singleton Engine
  
  if (Platform.isAndroid || Platform.isIOS) {
    Directory destDirectory = await getApplicationDocumentsDirectory();
    String wadPath = "${destDirectory.path}/doom1.wad";
    File file = File(wadPath);

    if (!file.existsSync()) {
      ByteData wad = await rootBundle.load("assets/doom1.wad");
      Uint8List wadBytes = wad.buffer.asUint8List(0);  
      await file.writeAsBytes(wadBytes, flush: true);
    }

    SystemChrome.setPreferredOrientations([
      DeviceOrientation.portraitUp,
      DeviceOrientation.portraitDown,
    ])
    .then((_) {
      runApp(MainApp(wadPath: wadPath));
    });
  }
  else if (Platform.isMacOS) {
    // TODO: TEMP
    runApp(MainApp(wadPath: "../Frameworks/App.framework/Resources/flutter_assets/assets/doom1.wad"));
  }
}

class MainApp extends StatelessWidget {
  final String wadPath;

  const MainApp({super.key, required this.wadPath});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      theme: (!Platform.isAndroid && !Platform.isIOS) ? null : ThemeData(
        fontFamily: 'BigBlue_TerminalPlus'
      ),
      builder: (!Platform.isAndroid && !Platform.isIOS) ? null : (context, child) {
        return AnnotatedRegion<SystemUiOverlayStyle>(
          value: const SystemUiOverlayStyle(statusBarIconBrightness: Brightness.light),
          child: child!
        );
      },
      home: Scaffold(
        backgroundColor: Platform.isAndroid || Platform.isIOS ? const Color.fromARGB(255, 15, 15, 15) : Colors.black,
        body: Platform.isAndroid || Platform.isIOS ?
          SafeArea(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.spaceAround,
              children: [
                Doom(wadPath: wadPath),
                PortraitKeyboard()
              ]
            )
          )
          :
          Doom(wadPath: wadPath)
      )
    );
  }
}