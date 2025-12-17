// input.hpp
#pragma once
#include <cstdint>
#include <string>

namespace input {

enum class Key : uint16_t {
  Unknown = 0,
  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z,
  Num0,
  Num1,
  Num2,
  Num3,
  Num4,
  Num5,
  Num6,
  Num7,
  Num8,
  Num9,
  Enter,
  Escape,
  Backspace,
  Tab,
  Space,
  Left,
  Right,
  Up,
  Down,
  F1,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  F11,
  F12,
};

enum Mod : uint8_t {
  Mod_None = 0,
  Mod_Shift = 1 << 0,
  Mod_Ctrl = 1 << 1,
  Mod_Alt = 1 << 2,
  Mod_Super = 1 << 3, // Cmd on macOS, Win key on Windows, Super on Linux
};

inline Mod operator|(Mod a, Mod b) {
  return static_cast<Mod>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline Mod &operator|=(Mod &a, Mod b) {
  a = a | b;
  return a;
}

inline bool operator&(Mod a, Mod b) {
  return (static_cast<uint8_t>(a) & static_cast<uint8_t>(b)) != 0;
}

struct KeyStroke {
  Key key{Key::Unknown};
  Mod mods{Mod_None};
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
  Capabilities capabilities() const;

  // Check if the backend is ready to inject events
  bool isReady() const;

  // Request necessary permissions (may show system dialogs)
  // Returns true if permissions are granted
  bool requestPermissions();

  // Key press/release
  bool keyDown(Key key, Mod mods = Mod_None);
  bool keyUp(Key key, Mod mods = Mod_None);

  // Combined key press + release
  bool tap(Key key, Mod mods = Mod_None);

  // Type arbitrary Unicode text
  bool typeText(const std::u32string &text);
  bool typeText(const std::string &utf8Text);

private:
  // Platform-specific implementation (pimpl idiom)
  struct Impl;
  Impl *m_impl{nullptr};
};

} // namespace input
