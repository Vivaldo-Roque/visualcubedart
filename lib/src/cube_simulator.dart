/// Simulates a 3×3 Rubik's Cube for sticker tracking.
///
/// Face layout (54 stickers):
/// ```
/// U: 0-8   L: 9-17   F: 18-26   R: 27-35   B: 36-44   D: 45-53
/// ```
class CubeSimulator {
  late List<int> stickers;

  CubeSimulator() {
    reset();
  }

  void reset() {
    stickers = List.generate(54, (i) => i);
  }

  /// Applies a standard algorithm string (e.g. `"R U R' U'"`) to the cube.
  void applyAlgorithm(String algorithm) {
    algorithm = algorithm.replaceAll('(', '').replaceAll(')', '');
    final moveRegex = RegExp(r"([UuFfRrDdLlBbMESxyz]w?)(['2]*)");
    for (final match in moveRegex.allMatches(algorithm)) {
      _applyMove(match.group(0)!);
    }
  }

  void _applyMove(String move) {
    int count = 1;
    if (move.contains("2")) {
      count = 2;
    } else if (move.endsWith("'") || move.contains("''")) {
      count = 3;
    }

    String face = move.replaceAll("2", "").replaceAll("'", "");

    for (int i = 0; i < count; i++) {
      _execute(face);
    }
  }

  void _execute(String move) {
    switch (move) {
      case 'U':
        _rotFace(0);
        _cycle([18, 9, 36, 27], [19, 10, 37, 28], [20, 11, 38, 29]);
        break;
      case 'D':
        _rotFace(45);
        _cycle([24, 33, 42, 15], [25, 34, 43, 16], [26, 35, 44, 17]);
        break;
      case 'L':
        _rotFace(9);
        _cycle([0, 18, 45, 44], [3, 21, 48, 41], [6, 24, 51, 38]);
        break;
      case 'R':
        _rotFace(27);
        _cycle([20, 2, 42, 47], [23, 5, 39, 50], [26, 8, 36, 53]);
        break;
      case 'F':
        _rotFace(18);
        _cycle([6, 27, 47, 17], [7, 30, 46, 14], [8, 33, 45, 11]);
        break;
      case 'B':
        _rotFace(36);
        _cycle([0, 15, 53, 29], [1, 12, 52, 32], [2, 9, 51, 35]);
        break;

      // Slice moves
      case 'M': // follows L direction
        _cycle([1, 19, 46, 43], [4, 22, 49, 40], [7, 25, 52, 37]);
        break;
      case 'E': // follows D direction
        _cycle([21, 30, 39, 12], [22, 31, 40, 13], [23, 32, 41, 14]);
        break;
      case 'S': // follows F direction
        _cycle([3, 30, 48, 12], [4, 31, 49, 13], [5, 32, 50, 14]);
        break;

      // Rotations
      case 'x':
        _execute('R');
        _rotPr('M');
        _rotPr('L');
        break;
      case 'y':
        _execute('U');
        _rotPr('E');
        _rotPr('D');
        break;
      case 'z':
        _execute('F');
        _execute('S');
        _rotPr('B');
        break;

      // Wide moves
      case 'u':
        _execute('U');
        _rotPr('E');
        break;
      case 'd':
        _execute('D');
        _execute('E');
        break;
      case 'l':
        _execute('L');
        _execute('M');
        break;
      case 'r':
        _execute('R');
        _rotPr('M');
        break;
      case 'f':
        _execute('F');
        _execute('S');
        break;
      case 'b':
        _execute('B');
        _rotPr('S');
        break;
    }
  }

  void _rotPr(String f) {
    for (int i = 0; i < 3; i++) {
      _execute(f);
    }
  }

  void _rotFace(int s) {
    _cycleSingle([s, s + 2, s + 8, s + 6]);
    _cycleSingle([s + 1, s + 5, s + 7, s + 3]);
  }

  void _cycle(List<int> a, [List<int>? b, List<int>? c]) {
    _cycleSingle(a);
    if (b != null) _cycleSingle(b);
    if (c != null) _cycleSingle(c);
  }

  void _cycleSingle(List<int> idx) {
    int temp = stickers[idx[3]];
    stickers[idx[3]] = stickers[idx[2]];
    stickers[idx[2]] = stickers[idx[1]];
    stickers[idx[1]] = stickers[idx[0]];
    stickers[idx[0]] = temp;
  }

  int stickerAt(int i) => stickers[i];
}
