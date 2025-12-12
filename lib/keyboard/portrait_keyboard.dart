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
    int asciiCode = 0;

    bool isSpecialKey = event.logicalKey.keyId & 0xF00000000 > 0;

    if (!isSpecialKey) {
      print("DEFAULT");
      asciiCode = event.logicalKey.keyId;
    }
    else {
      print("SPECIAL");
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
          asciiCode = AsciiKeys.keyCodes["KEY_ESCAPE"]!;
          break;

        case "Enter":
        case "Numpad Enter":
          asciiCode = AsciiKeys.keyCodes["KEY_ENTER"]!;
          break;

        case "Tab":
          asciiCode = AsciiKeys.keyCodes["KEY_TAB"]!;
          break;

        case "Control Right":
          asciiCode = AsciiKeys.keyCodes["KEY_RCTRL"]!;
          break;


        /*"KEY_F1": 0x80+0x3b,
        "KEY_F2": 0x80+0x3c,
        "KEY_F3": 0x80+0x3d,
        "KEY_F4": 0x80+0x3e,
        "KEY_F5": 0x80+0x3f,
        "KEY_F6": 0x80+0x40,
        "KEY_F7": 0x80+0x41,
        "KEY_F8": 0x80+0x42,
        "KEY_F9": 0x80+0x43,
        "KEY_F10": 0x80+0x44,
        "KEY_F11": 0x80+0x57,
        "KEY_F12": 0x80+0x58,
        "KEY_BACKSPACE": 127,
        "KEY_PAUSE": 0xff,
        "KEY_EQUALS": 0x3d,
        "KEY_MINUS": 0x2d,
        "KEY_RSHIFT": 0x80+0x36,
        "KEY_RCTRL": 0x80+0x1d,
        "KEY_RALT": 0x80+0x38,
        "KEY_LALT": 0x80+0x38,
        ",": 44,
        ".": 46,
        "SPACE": 32,
        "1": 49,
        "2": 50,
        "3": 51,
        "4": 52,
        "5": 53,
        "6": 54,
        "7": 55,
        "Y": 121*/
      }
    }

    print(event);

    engine.dartPostInput(asciiCode, event is KeyDownEvent || event is KeyRepeatEvent ? 1 : 0);
    return KeyEventResult.handled;



    /*if (event is KeyDownEvent) {
      debugPrint('Focus node ${node.debugLabel} got key event: ${event.logicalKey}');
      switch (event.logicalKey) {
        case LogicalKeyboardKey.keyR:
          debugPrint('Changing color to red.');
          setState(() {
            _color = Colors.red;
          });
          return KeyEventResult.handled;
        case LogicalKeyboardKey.keyG:
          debugPrint('Changing color to green.');
          setState(() {
            _color = Colors.green;
          });
          return KeyEventResult.handled;
        case LogicalKeyboardKey.keyB:
          debugPrint('Changing color to blue.');
          setState(() {
            _color = Colors.blue;
          });
          return KeyEventResult.handled;
      }
    }*/

    print(event);
    return KeyEventResult.ignored;
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