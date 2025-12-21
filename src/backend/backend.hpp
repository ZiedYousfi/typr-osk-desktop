#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace backend {

enum class Key : uint16_t {
  Unknown = 0,
  // Letters
  A = 1, B, C, D, E, F, G, H, I, J, K, L, M,
  N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
  // Numbers (main row)
  Num0 = 33, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
  // Function keys
  F1 = 43, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
  F13, F14, F15, F16, F17, F18, F19, F20,
  // Control keys
  Enter = 63, Escape, Backspace, Tab, Space,
  // Navigation
  Left = 68, Right, Up, Down, Home, End, PageUp, PageDown,
  Delete, Insert, PrintScreen, ScrollLock, Pause,
  // Numpad
  NumpadDivide = 83, NumpadMultiply, NumpadMinus, NumpadPlus,
  NumpadEnter, NumpadDecimal,
  Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
  Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
  // Modifiers
  ShiftLeft = 99, ShiftRight,
  CtrlLeft, CtrlRight,
  AltLeft, AltRight,
  SuperLeft, SuperRight,
  CapsLock, NumLock,
  // Misc
  Help = 109, Menu, Power, Sleep, Wake,
  Mute, VolumeDown, VolumeUp,
  MediaPlayPause, MediaStop, MediaNext, MediaPrevious,
  BrightnessDown, BrightnessUp, Eject,
  // Punctuation (layout-dependent position)
  Grave = 124, Minus, Equal,
  LeftBracket, RightBracket, Backslash,
  Semicolon, Apostrophe, Comma, Period, Slash,
};

// Active modifier state (bitmask)
enum class Modifier : uint8_t {
  None = 0,
  Shift = 1 << 0,
  Ctrl = 1 << 1,
  Alt = 1 << 2,
  Super = 1 << 3,
  CapsLock = 1 << 4,
  NumLock = 1 << 5,
};

inline Modifier operator|(Modifier a, Modifier b) {
  return static_cast<Modifier>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
inline Modifier operator&(Modifier a, Modifier b) {
  return static_cast<Modifier>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
inline bool hasModifier(Modifier state, Modifier flag) {
  return (static_cast<uint8_t>(state) & static_cast<uint8_t>(flag)) != 0;
}

struct Capabilities {
  bool canInjectKeys{false};
  bool canInjectText{false};
  bool canSimulateHID{false};       // True hardware-level simulation
  bool supportsKeyRepeat{false};    // OS handles repeat automatically
  bool needsAccessibilityPerm{false};
  bool needsInputMonitoringPerm{false};
  bool needsUinputAccess{false};    // Linux: /dev/uinput
};

// Backend type detection
enum class BackendType {
  Unknown,
  Windows,
  MacOS,
  LinuxX11,
  LinuxWayland,
  LinuxUInput,  // Direct uinput (works everywhere on Linux)
};

class InputBackend {
public:
  InputBackend();
  ~InputBackend();

  // Non-copyable, movable
  InputBackend(const InputBackend&) = delete;
  InputBackend& operator=(const InputBackend&) = delete;
  InputBackend(InputBackend&&) noexcept;
  InputBackend& operator=(InputBackend&&) noexcept;

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
  bool typeText(const std::u32string& text);
  bool typeText(const std::string& utf8Text);
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

// Utility functions
std::string keyToString(Key key);
Key stringToKey(const std::string& str);

} // namespace backend