#pragma once

#include <cstdint>
#include <string>

namespace backend {

enum class Key : uint16_t {
  Unknown = 0,
  A = 1,
  B = 2,
  C = 3,
  D = 4,
  E = 5,
  F = 6,
  G = 7,
  H = 8,
  I = 9,
  J = 10,
  K = 11,
  L = 12,
  M = 13,
  N = 14,
  O = 15,
  P = 16,
  Q = 17,
  R = 18,
  S = 19,
  T = 20,
  U = 21,
  V = 22,
  W = 23,
  X = 24,
  Y = 25,
  Z = 26,
  Num0 = 33,
  Num1 = 34,
  Num2 = 35,
  Num3 = 36,
  Num4 = 37,
  Num5 = 38,
  Num6 = 39,
  Num7 = 40,
  Num8 = 41,
  Num9 = 42,
  F1 = 43,
  F2 = 44,
  F3 = 45,
  F4 = 46,
  F5 = 47,
  F6 = 48,
  F7 = 49,
  F8 = 50,
  F9 = 51,
  F10 = 52,
  F11 = 53,
  F12 = 54,
  F13 = 55,
  F14 = 56,
  F15 = 57,
  F16 = 58,
  F17 = 59,
  F18 = 60,
  F19 = 61,
  F20 = 62,
  Enter = 63,
  Escape = 64,
  Backspace = 65,
  Tab = 66,
  Space = 67,
  Left = 68,
  Right = 69,
  Up = 70,
  Down = 71,
  Home = 72,
  End = 73,
  PageUp = 74,
  PageDown = 75,
  Delete = 76,
  Insert = 77,
  PrintScreen = 78,
  ScrollLock = 79,
  Pause = 80,
  NumpadDivide = 83,
  NumpadMultiply = 84,
  NumpadMinus = 85,
  NumpadPlus = 86,
  NumpadEnter = 87,
  NumpadDecimal = 88,
  Numpad0 = 89,
  Numpad1 = 90,
  Numpad2 = 91,
  Numpad3 = 92,
  Numpad4 = 93,
  Numpad5 = 94,
  Numpad6 = 95,
  Numpad7 = 96,
  Numpad8 = 97,
  Numpad9 = 98,
  ShiftLeft = 99,
  ShiftRight = 100,
  CtrlLeft = 101,
  CtrlRight = 102,
  AltLeft = 103,
  AltRight = 104,
  SuperLeft = 105,
  SuperRight = 106,
  CapsLock = 107,
  NumLock = 108,
  Help = 109,
  Menu = 110,
  Power = 111,
  Sleep = 112,
  Wake = 113,
  Mute = 114,
  VolumeDown = 115,
  VolumeUp = 116,
  MediaPlayPause = 117,
  MediaStop = 118,
  MediaNext = 119,
  MediaPrevious = 120,
  BrightnessDown = 121,
  BrightnessUp = 122,
  Eject = 123,
  Grave = 124,
  Minus = 125,
  Equal = 126,
  LeftBracket = 127,
  RightBracket = 128,
  Backslash = 129,
  Semicolon = 130,
  Apostrophe = 131,
  Comma = 132,
  Period = 133,
  Slash = 134,
  CharacterInput = 0xFFFE,
  BackspaceDelete = 0xFFFF,
};

inline std::string keyToString(Key key) {
  switch (key) {
  case Key::A:
    return "A";
  case Key::B:
    return "B";
  case Key::C:
    return "C";
  case Key::D:
    return "D";
  case Key::E:
    return "E";
  case Key::F:
    return "F";
  case Key::G:
    return "G";
  case Key::H:
    return "H";
  case Key::I:
    return "I";
  case Key::J:
    return "J";
  case Key::K:
    return "K";
  case Key::L:
    return "L";
  case Key::M:
    return "M";
  case Key::N:
    return "N";
  case Key::O:
    return "O";
  case Key::P:
    return "P";
  case Key::Q:
    return "Q";
  case Key::R:
    return "R";
  case Key::S:
    return "S";
  case Key::T:
    return "T";
  case Key::U:
    return "U";
  case Key::V:
    return "V";
  case Key::W:
    return "W";
  case Key::X:
    return "X";
  case Key::Y:
    return "Y";
  case Key::Z:
    return "Z";
  case Key::Num0:
    return "0";
  case Key::Num1:
    return "1";
  case Key::Num2:
    return "2";
  case Key::Num3:
    return "3";
  case Key::Num4:
    return "4";
  case Key::Num5:
    return "5";
  case Key::Num6:
    return "6";
  case Key::Num7:
    return "7";
  case Key::Num8:
    return "8";
  case Key::Num9:
    return "9";
  case Key::F1:
    return "F1";
  case Key::F2:
    return "F2";
  case Key::F3:
    return "F3";
  case Key::F4:
    return "F4";
  case Key::F5:
    return "F5";
  case Key::F6:
    return "F6";
  case Key::F7:
    return "F7";
  case Key::F8:
    return "F8";
  case Key::F9:
    return "F9";
  case Key::F10:
    return "F10";
  case Key::F11:
    return "F11";
  case Key::F12:
    return "F12";
  case Key::F13:
    return "F13";
  case Key::F14:
    return "F14";
  case Key::F15:
    return "F15";
  case Key::F16:
    return "F16";
  case Key::F17:
    return "F17";
  case Key::F18:
    return "F18";
  case Key::F19:
    return "F19";
  case Key::F20:
    return "F20";
  case Key::Enter:
    return "Return";
  case Key::Escape:
    return "Esc";
  case Key::Backspace:
    return "Delete";
  case Key::Tab:
    return "Tab";
  case Key::Space:
    return "Space";
  case Key::Left:
    return "←";
  case Key::Right:
    return "→";
  case Key::Up:
    return "↑";
  case Key::Down:
    return "↓";
  case Key::Home:
    return "Home";
  case Key::End:
    return "End";
  case Key::PageUp:
    return "PageUp";
  case Key::PageDown:
    return "PageDown";
  case Key::Delete:
    return "Delete";
  case Key::Insert:
    return "Insert";
  case Key::PrintScreen:
    return "PrintScreen";
  case Key::ScrollLock:
    return "ScrollLock";
  case Key::Pause:
    return "Pause";
  case Key::NumpadDivide:
    return "NumpadDivide";
  case Key::NumpadMultiply:
    return "NumpadMultiply";
  case Key::NumpadMinus:
    return "NumpadMinus";
  case Key::NumpadPlus:
    return "NumpadPlus";
  case Key::NumpadEnter:
    return "NumpadEnter";
  case Key::NumpadDecimal:
    return "NumpadDecimal";
  case Key::Numpad0:
    return "Numpad0";
  case Key::Numpad1:
    return "Numpad1";
  case Key::Numpad2:
    return "Numpad2";
  case Key::Numpad3:
    return "Numpad3";
  case Key::Numpad4:
    return "Numpad4";
  case Key::Numpad5:
    return "Numpad5";
  case Key::Numpad6:
    return "Numpad6";
  case Key::Numpad7:
    return "Numpad7";
  case Key::Numpad8:
    return "Numpad8";
  case Key::Numpad9:
    return "Numpad9";
  case Key::ShiftLeft:
  case Key::ShiftRight:
    return "Shift";
  case Key::CtrlLeft:
  case Key::CtrlRight:
    return "Ctrl";
  case Key::AltLeft:
  case Key::AltRight:
    return "Opt";
  case Key::SuperLeft:
  case Key::SuperRight:
    return "Cmd";
  case Key::CapsLock:
    return "CapsLock";
  case Key::NumLock:
    return "NumLock";
  case Key::Help:
    return "Help";
  case Key::Menu:
    return "Menu";
  case Key::Power:
    return "Power";
  case Key::Sleep:
    return "Sleep";
  case Key::Wake:
    return "Wake";
  case Key::Mute:
    return "Mute";
  case Key::VolumeDown:
    return "VolumeDown";
  case Key::VolumeUp:
    return "VolumeUp";
  case Key::MediaPlayPause:
    return "MediaPlayPause";
  case Key::MediaStop:
    return "MediaStop";
  case Key::MediaNext:
    return "MediaNext";
  case Key::MediaPrevious:
    return "MediaPrevious";
  case Key::BrightnessDown:
    return "BrightnessDown";
  case Key::BrightnessUp:
    return "BrightnessUp";
  case Key::Eject:
    return "Eject";
  case Key::Grave:
    return "`";
  case Key::Minus:
    return "-";
  case Key::Equal:
    return "=";
  case Key::LeftBracket:
    return "[";
  case Key::RightBracket:
    return "]";
  case Key::Backslash:
    return "\\";
  case Key::Semicolon:
    return ";";
  case Key::Apostrophe:
    return "'";
  case Key::Comma:
    return ",";
  case Key::Period:
    return ".";
  case Key::Slash:
    return "/";
  case Key::CharacterInput:
    return "CharacterInput";
  case Key::BackspaceDelete:
    return "BackspaceDelete";
  case Key::Unknown:
  default:
    return "Unknown";
  }
}

struct KeyStroke {
  Key key{Key::Unknown};
};

struct Capabilities {
  bool canInjectEvents{false}; // "soft" injection via OS APIs
  bool canActAsHID{false};     // "hard" injection as HID device
  bool needsAccessibilityPerm{false};
  bool needsInputMonitoringPerm{false};
};

// Single class with platform-specific implementation
// Users only need to include this header and use InputBackend directly
class InputBackend {
public:
  InputBackend();
  ~InputBackend();

  // Non-copyable, movable
  InputBackend(const InputBackend &) = delete;
  InputBackend &operator=(const InputBackend &) = delete;
  InputBackend(InputBackend &&) noexcept;
  InputBackend &operator=(InputBackend &&) noexcept;

  // Query capabilities of this backend
  [[nodiscard]] Capabilities capabilities() const;

  // Check if the backend is ready to inject events
  [[nodiscard]] bool isReady() const;

  // Request necessary permissions (may show system dialogs)
  // Returns true if permissions are granted
  bool requestPermissions();

  // Key press/release with optional character data
  // If keystroke.character is set, it will inject that Unicode character
  bool keyDown(const KeyStroke &keystroke);
  bool keyUp(const KeyStroke &keystroke);

  // Combined key press + release
  bool tap(const KeyStroke &keystroke);

  // Type arbitrary Unicode text (any character in any language)
  bool typeText(const std::u32string &text);
  bool typeText(const std::string &utf8Text);

  // Type a single Unicode character
  bool typeCharacter(char32_t codepoint);

private:
  // Platform-specific implementation (pimpl idiom)
  struct Impl;
  Impl *m_impl{nullptr};
};

} // namespace backend
