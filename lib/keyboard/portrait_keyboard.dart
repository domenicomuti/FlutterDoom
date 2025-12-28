import 'package:flutter/material.dart';
import 'package:flutterdoom/engine.dart';
import 'package:flutterdoom/keyboard/bottom_keys.dart';
import 'package:flutterdoom/keyboard/directional_keys.dart';
import 'package:flutterdoom/keyboard/fire_key.dart';
import 'package:flutterdoom/keyboard/top_keys.dart';

class PortraitKeyboard extends StatefulWidget {
  const PortraitKeyboard({super.key});

  @override
  State<PortraitKeyboard> createState() => _PortraitKeyboardState();  
}

class _PortraitKeyboardState extends State<PortraitKeyboard> {
  Engine engine = Engine();

  @override
  void initState() {
    super.initState();
  }

  @override
  void dispose() {
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Expanded(child: Padding(
      padding: EdgeInsets.symmetric(horizontal: MediaQuery.of(context).size.width * 0.05),
      child: Column(
          mainAxisAlignment: MainAxisAlignment.spaceAround,
          children: [
            SystemKeys(),
            NumericKeys(),
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                DirectionalKeys(),
                FireKey()
              ]
            ),
            BottomKeys()
          ]
        )
      )
    );
  }
}