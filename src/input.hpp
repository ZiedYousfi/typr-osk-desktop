// input.hpp
#pragma once
#include <cstdint>
#include <memory>
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
  Mod_Super = 1 << 3, // Cmd on macOS, Win key on Windows, Super key on Linux
                      // (ur welcome :))
};

inline Mod operator|(Mod a, Mod b) {
  return static_cast<Mod>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

struct KeyStroke {
  Key key{Key::Unknown};
  Mod mods{Mod_None};
};

struct Capabilities {
  bool canInjectEvents{false}; // "soft"
  bool canActAsHID{false};     // "hard"
  bool needsAccessibilityPerm{false};
  bool needsInputMonitoringPerm{false};
};

class IInputBackend {
public:
  virtual ~IInputBackend() = default;

  virtual Capabilities capabilities() const = 0;

  // Soft injection style
  virtual bool keyDown(Key key, Mod mods) = 0;
  virtual bool keyUp(Key key, Mod mods) = 0;

  virtual bool tap(Key key, Mod mods) {
    return keyDown(key, mods) && keyUp(key, mods);
  }

  // "Type" is intentionally higher-level
  virtual bool typeText(const std::u32string &text) = 0;

  // Optional: for future HID backend
  virtual bool connectHID() { return false; }
  virtual bool disconnectHID() { return false; }
};

std::unique_ptr<IInputBackend> makeDefaultBackend();

} // namespace input
