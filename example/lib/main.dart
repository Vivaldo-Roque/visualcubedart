import 'package:flutter/material.dart';
import 'package:visualcubedart/visualcubedart.dart' as visualcubedart;

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String? _svgOutput;

  @override
  void initState() {
    super.initState();
    _svgOutput = visualcubedart.generateCubeSvg(
      dim: 3,
      size: 128,
      view: 'normal',
      co: 100,
      fo: 100,
      rotX: -33,
      rotY: 33,
      rotZ: 18,
    );
  }

  @override
  Widget build(BuildContext context) {
    const textStyle = TextStyle(fontSize: 20);
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('visualcubedart Example')),
        body: Center(
          child: Padding(
            padding: const EdgeInsets.all(16.0),
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Text(
                  _svgOutput != null
                      ? 'Successfully generated Rubik\'s Cube SVG!'
                      : 'Failed to generate Rubik\'s Cube SVG.',
                  style: textStyle,
                  textAlign: TextAlign.center,
                ),
                const SizedBox(height: 20),
                if (_svgOutput != null)
                  Expanded(
                    child: SingleChildScrollView(
                      child: Text(
                        _svgOutput!,
                        style: const TextStyle(fontFamily: 'monospace', fontSize: 10),
                      ),
                    ),
                  ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
