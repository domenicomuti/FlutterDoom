import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutterdoom/engine.dart';
import 'package:flutterdoom/keyboard/ascii_keys.dart';
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
  late FocusNode _node;
  late FocusAttachment _nodeAttachment;
  Engine engine = Engine();

  @override
  void initState() {
    super.initState();
    _node = FocusNode(debugLabel: 'Button');
    _node.requestFocus();
    _nodeAttachment = _node.attach(context, onKeyEvent: _handleKeyPress);
  }

  @override
  void dispose() {
    _node.dispose();
    super.dispose();
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

  @override
  Widget build(BuildContext context) {
    _nodeAttachment.reparent();

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