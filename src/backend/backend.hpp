#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace backend {

enum class Key : uint8_t {
  Unknown = 0,
  // Letters
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
  // Numbers (main row)
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
  // Function keys
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
  // Control keys
  Enter = 63,
  Escape = 64,
  Backspace = 65,
  Tab = 66,
  Space = 67,
  // Navigation
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
  // Numpad
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
  // Modifiers
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
  // Misc
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
  // Punctuation (layout-dependent position)
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
};

// Active modifier state (bitmask)
enum class Modifier : uint8_t {
  None = 0,
  Shift = 0x01,
  Ctrl = 0x02,
  Alt = 0x04,
  Super = 0x08,
  CapsLock = 0x10,
  NumLock = 0x20,
};

inline Modifier operator|(Modifier first, Modifier second) {
  return static_cast<Modifier>(static_cast<uint8_t>(first) |
                               static_cast<uint8_t>(second));
}
inline Modifier operator&(Modifier first, Modifier second) {
  return static_cast<Modifier>(static_cast<uint8_t>(first) &
                               static_cast<uint8_t>(second));
}
inline bool hasModifier(Modifier state, Modifier flag) {
  return (static_cast<uint8_t>(state) & static_cast<uint8_t>(flag)) != 0;
}

struct Capabilities {
  bool canInjectKeys{false};
  bool canInjectText{false};
  bool canSimulateHID{false};    // True hardware-level simulation
  bool supportsKeyRepeat{false}; // OS handles repeat automatically
  bool needsAccessibilityPerm{false};
  bool needsInputMonitoringPerm{false};
  bool needsUinputAccess{false}; // Linux: /dev/uinput
};

// Backend type detection
enum class BackendType : uint8_t {
  Unknown,
  Windows,
  MacOS,
  LinuxX11,
  LinuxWayland,
  LinuxUInput, // Direct uinput (works everywhere on Linux)
};

class InputBackend {
public:
  InputBackend();
  ~InputBackend();

  // Non-copyable, movable
  InputBackend(const InputBackend &) = delete;
  InputBackend &operator=(const InputBackend &) = delete;
  InputBackend(InputBackend &&) noexcept;
  InputBackend &operator=(InputBackend &&) noexcept;

  // --- Info ---
  [[nodiscard]] BackendType type() const;
  [[nodiscard]] Capabilities capabilities() const;
  [[nodiscard]] bool isReady() const;
  bool requestPermissions();

  // --- Physical Key Events ---
  // These simulate actual hardware key presses.
  // The key stays "pressed" until you call keyUp().
  // OS will generate key repeat automatically after the repeat delay.
  bool keyDown(Key key);
  bool keyUp(Key key);

  // Convenience: keyDown + small delay + keyUp
  bool tap(Key key);

  // --- Modifier Helpers ---
  // Returns currently held modifiers (from our own state tracking)
  [[nodiscard]] Modifier activeModifiers() const;

  // Press/release modifier keys
  bool holdModifier(Modifier mod);
  bool releaseModifier(Modifier mod);
  bool releaseAllModifiers();

  // Execute a key combo: holds modifiers, taps key, releases modifiers
  // Example: combo(Modifier::Ctrl | Modifier::Shift, Key::S)
  bool combo(Modifier mods, Key key);

  // --- Text Input ---
  // Injects Unicode text directly (layout-independent).
  // Does NOT trigger physical key events - just inserts characters.
  bool typeText(const std::u32string &text);
  bool typeText(const std::string &utf8Text);
  bool typeCharacter(char32_t codepoint);

  // --- Advanced ---
  // Force sync/flush pending events (some backends buffer)
  void flush();

  // Set delay between key events in tap/combo (microseconds)
  void setKeyDelay(uint32_t delayUs);

private:
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

// OutputListener: listens to global keyboard events (keys down/up and produced
// Unicode output).
class OutputListener {
public:
  // Callback invoked for each key event. Parameters:
  //  - Unicode codepoint produced by the event (0 if none or non-printable)
  //  - Mapped physical Key (Key::Unknown if unknown)
  //  - Current modifier state
  //  - true for key press, false for key release
  using Callback = std::function<void(char32_t codepoint, Key key,
                                      Modifier mods, bool pressed)>;

  OutputListener();
  ~OutputListener();

  OutputListener(const OutputListener &) = delete;
  OutputListener &operator=(const OutputListener &) = delete;
  OutputListener(OutputListener &&) noexcept;
  OutputListener &operator=(OutputListener &&) noexcept;

  // Start listening to global keyboard events. Returns true on success.
  bool startListening(Callback cb);
  // Stop listening. Safe to call from any thread.
  void stopListening();
  // Whether the listener is currently active.
  [[nodiscard]] bool isListening() const;

private:
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

// Utility functions
std::string keyToString(Key key);
Key stringToKey(const std::string &str);

} // namespace backend
