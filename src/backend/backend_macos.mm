#ifdef __APPLE__

#include "backend.hpp"

#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#import <Foundation/Foundation.h>
#include <thread>
#include <chrono>
#include <unordered_map>

namespace backend {

namespace {

CGKeyCode keyToMacKeyCode(Key key) {
  static const std::unordered_map<Key, CGKeyCode> map = {
    // Letters
    {Key::A, kVK_ANSI_A}, {Key::B, kVK_ANSI_B}, {Key::C, kVK_ANSI_C},
    {Key::D, kVK_ANSI_D}, {Key::E, kVK_ANSI_E}, {Key::F, kVK_ANSI_F},
    {Key::G, kVK_ANSI_G}, {Key::H, kVK_ANSI_H}, {Key::I, kVK_ANSI_I},
    {Key::J, kVK_ANSI_J}, {Key::K, kVK_ANSI_K}, {Key::L, kVK_ANSI_L},
    {Key::M, kVK_ANSI_M}, {Key::N, kVK_ANSI_N}, {Key::O, kVK_ANSI_O},
    {Key::P, kVK_ANSI_P}, {Key::Q, kVK_ANSI_Q}, {Key::R, kVK_ANSI_R},
    {Key::S, kVK_ANSI_S}, {Key::T, kVK_ANSI_T}, {Key::U, kVK_ANSI_U},
    {Key::V, kVK_ANSI_V}, {Key::W, kVK_ANSI_W}, {Key::X, kVK_ANSI_X},
    {Key::Y, kVK_ANSI_Y}, {Key::Z, kVK_ANSI_Z},
    // Numbers
    {Key::Num0, kVK_ANSI_0}, {Key::Num1, kVK_ANSI_1}, {Key::Num2, kVK_ANSI_2},
    {Key::Num3, kVK_ANSI_3}, {Key::Num4, kVK_ANSI_4}, {Key::Num5, kVK_ANSI_5},
    {Key::Num6, kVK_ANSI_6}, {Key::Num7, kVK_ANSI_7}, {Key::Num8, kVK_ANSI_8},
    {Key::Num9, kVK_ANSI_9},
    // Function keys
    {Key::F1, kVK_F1}, {Key::F2, kVK_F2}, {Key::F3, kVK_F3}, {Key::F4, kVK_F4},
    {Key::F5, kVK_F5}, {Key::F6, kVK_F6}, {Key::F7, kVK_F7}, {Key::F8, kVK_F8},
    {Key::F9, kVK_F9}, {Key::F10, kVK_F10}, {Key::F11, kVK_F11}, {Key::F12, kVK_F12},
    {Key::F13, kVK_F13}, {Key::F14, kVK_F14}, {Key::F15, kVK_F15}, {Key::F16, kVK_F16},
    {Key::F17, kVK_F17}, {Key::F18, kVK_F18}, {Key::F19, kVK_F19}, {Key::F20, kVK_F20},
    // Control
    {Key::Enter, kVK_Return}, {Key::Escape, kVK_Escape}, {Key::Backspace, kVK_Delete},
    {Key::Tab, kVK_Tab}, {Key::Space, kVK_Space},
    // Navigation
    {Key::Left, kVK_LeftArrow}, {Key::Right, kVK_RightArrow},
    {Key::Up, kVK_UpArrow}, {Key::Down, kVK_DownArrow},
    {Key::Home, kVK_Home}, {Key::End, kVK_End},
    {Key::PageUp, kVK_PageUp}, {Key::PageDown, kVK_PageDown},
    {Key::Delete, kVK_ForwardDelete}, {Key::Insert, kVK_Help},
    // Numpad
    {Key::Numpad0, kVK_ANSI_Keypad0}, {Key::Numpad1, kVK_ANSI_Keypad1},
    {Key::Numpad2, kVK_ANSI_Keypad2}, {Key::Numpad3, kVK_ANSI_Keypad3},
    {Key::Numpad4, kVK_ANSI_Keypad4}, {Key::Numpad5, kVK_ANSI_Keypad5},
    {Key::Numpad6, kVK_ANSI_Keypad6}, {Key::Numpad7, kVK_ANSI_Keypad7},
    {Key::Numpad8, kVK_ANSI_Keypad8}, {Key::Numpad9, kVK_ANSI_Keypad9},
    {Key::NumpadDivide, kVK_ANSI_KeypadDivide},
    {Key::NumpadMultiply, kVK_ANSI_KeypadMultiply},
    {Key::NumpadMinus, kVK_ANSI_KeypadMinus},
    {Key::NumpadPlus, kVK_ANSI_KeypadPlus},
    {Key::NumpadEnter, kVK_ANSI_KeypadEnter},
    {Key::NumpadDecimal, kVK_ANSI_KeypadDecimal},
    // Modifiers
    {Key::ShiftLeft, kVK_Shift}, {Key::ShiftRight, kVK_RightShift},
    {Key::CtrlLeft, kVK_Control}, {Key::CtrlRight, kVK_RightControl},
    {Key::AltLeft, kVK_Option}, {Key::AltRight, kVK_RightOption},
    {Key::SuperLeft, kVK_Command}, {Key::SuperRight, kVK_RightCommand},
    {Key::CapsLock, kVK_CapsLock}, {Key::NumLock, kVK_ANSI_KeypadClear},
    // Misc
    {Key::Mute, kVK_Mute}, {Key::VolumeDown, kVK_VolumeDown}, {Key::VolumeUp, kVK_VolumeUp},
    // Punctuation
    {Key::Grave, kVK_ANSI_Grave}, {Key::Minus, kVK_ANSI_Minus}, {Key::Equal, kVK_ANSI_Equal},
    {Key::LeftBracket, kVK_ANSI_LeftBracket}, {Key::RightBracket, kVK_ANSI_RightBracket},
    {Key::Backslash, kVK_ANSI_Backslash}, {Key::Semicolon, kVK_ANSI_Semicolon},
    {Key::Apostrophe, kVK_ANSI_Quote}, {Key::Comma, kVK_ANSI_Comma},
    {Key::Period, kVK_ANSI_Period}, {Key::Slash, kVK_ANSI_Slash},
  };
  
  auto it = map.find(key);
  return (it != map.end()) ? it->second : UINT16_MAX;
}

CGEventFlags modifierToFlags(Modifier mod) {
  CGEventFlags flags = 0;
  if (hasModifier(mod, Modifier::Shift)) flags |= kCGEventFlagMaskShift;
  if (hasModifier(mod, Modifier::Ctrl)) flags |= kCGEventFlagMaskControl;
  if (hasModifier(mod, Modifier::Alt)) flags |= kCGEventFlagMaskAlternate;
  if (hasModifier(mod, Modifier::Super)) flags |= kCGEventFlagMaskCommand;
  if (hasModifier(mod, Modifier::CapsLock)) flags |= kCGEventFlagMaskAlphaShift;
  return flags;
}

} // namespace

struct InputBackend::Impl {
  CGEventSourceRef eventSource{nullptr};
  Modifier currentMods{Modifier::None};
  uint32_t keyDelayUs{1000};
  bool ready{false};

  Impl() {
    eventSource = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    ready = AXIsProcessTrustedWithOptions(nullptr);
  }

  ~Impl() {
    if (eventSource) {
      CFRelease(eventSource);
    }
  }

  bool sendKey(Key key, bool down) {
    CGKeyCode keyCode = keyToMacKeyCode(key);
    if (keyCode == UINT16_MAX) return false;

    CGEventRef event = CGEventCreateKeyboardEvent(eventSource, keyCode, down);
    if (!event) return false;

    // Apply current modifier state
    CGEventSetFlags(event, modifierToFlags(currentMods));
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
    return true;
  }

  bool typeUnicode(const std::u32string& text) {
    if (text.empty()) return true;

    // Convert to UTF-16
    std::vector<UniChar> utf16;
    for (char32_t cp : text) {
      if (cp <= 0xFFFF) {
        utf16.push_back(static_cast<UniChar>(cp));
      } else if (cp <= 0x10FFFF) {
        cp -= 0x10000;
        utf16.push_back(static_cast<UniChar>(0xD800 | (cp >> 10)));
        utf16.push_back(static_cast<UniChar>(0xDC00 | (cp & 0x3FF)));
      }
    }

    // macOS limit: 20 characters per event
    constexpr size_t kMaxChars = 20;

    for (size_t i = 0; i < utf16.size(); i += kMaxChars) {
      size_t len = std::min(kMaxChars, utf16.size() - i);

      CGEventRef down = CGEventCreateKeyboardEvent(eventSource, 0, true);
      CGEventRef up = CGEventCreateKeyboardEvent(eventSource, 0, false);
      if (!down || !up) {
        if (down) CFRelease(down);
        if (up) CFRelease(up);
        return false;
      }

      CGEventKeyboardSetUnicodeString(down, len, utf16.data() + i);
      CGEventKeyboardSetUnicodeString(up, len, utf16.data() + i);

      CGEventPost(kCGHIDEventTap, down);
      CGEventPost(kCGHIDEventTap, up);

      CFRelease(down);
      CFRelease(up);
    }
    return true;
  }

  void delay() {
    if (keyDelayUs > 0) {
      std::this_thread::sleep_for(std::chrono::microseconds(keyDelayUs));
    }
  }
};

InputBackend::InputBackend() : m_impl(std::make_unique<Impl>()) {}
InputBackend::~InputBackend() = default;
InputBackend::InputBackend(InputBackend&&) noexcept = default;
InputBackend& InputBackend::operator=(InputBackend&&) noexcept = default;

BackendType InputBackend::type() const { return BackendType::MacOS; }

Capabilities InputBackend::capabilities() const {
  return {
    .canInjectKeys = m_impl->ready,
    .canInjectText = m_impl->ready,
    .canSimulateHID = false,  // macOS CGEvent is not true HID
    .supportsKeyRepeat = true,
    .needsAccessibilityPerm = true,
    .needsInputMonitoringPerm = false,
    .needsUinputAccess = false,
  };
}

bool InputBackend::isReady() const { return m_impl->ready; }

bool InputBackend::requestPermissions() {
  NSDictionary* opts = @{(__bridge NSString*)kAXTrustedCheckOptionPrompt: @YES};
  m_impl->ready = AXIsProcessTrustedWithOptions((__bridge CFDictionaryRef)opts);
  return m_impl->ready;
}

bool InputBackend::keyDown(Key key) {
  // Update modifier state if pressing a modifier
  switch (key) {
    case Key::ShiftLeft: case Key::ShiftRight:
      m_impl->currentMods = m_impl->currentMods | Modifier::Shift; break;
    case Key::CtrlLeft: case Key::CtrlRight:
      m_impl->currentMods = m_impl->currentMods | Modifier::Ctrl; break;
    case Key::AltLeft: case Key::AltRight:
      m_impl->currentMods = m_impl->currentMods | Modifier::Alt; break;
    case Key::SuperLeft: case Key::SuperRight:
      m_impl->currentMods = m_impl->currentMods | Modifier::Super; break;
    default: break;
  }
  return m_impl->sendKey(key, true);
}

bool InputBackend::keyUp(Key key) {
  bool result = m_impl->sendKey(key, false);
  // Update modifier state if releasing a modifier
  switch (key) {
    case Key::ShiftLeft: case Key::ShiftRight:
      m_impl->currentMods = static_cast<Modifier>(
        static_cast<uint8_t>(m_impl->currentMods) & ~static_cast<uint8_t>(Modifier::Shift));
      break;
    case Key::CtrlLeft: case Key::CtrlRight:
      m_impl->currentMods = static_cast<Modifier>(
        static_cast<uint8_t>(m_impl->currentMods) & ~static_cast<uint8_t>(Modifier::Ctrl));
      break;
    case Key::AltLeft: case Key::AltRight:
      m_impl->currentMods = static_cast<Modifier>(
        static_cast<uint8_t>(m_impl->currentMods) & ~static_cast<uint8_t>(Modifier::Alt));
      break;
    case Key::SuperLeft: case Key::SuperRight:
      m_impl->currentMods = static_cast<Modifier>(
        static_cast<uint8_t>(m_impl->currentMods) & ~static_cast<uint8_t>(Modifier::Super));
      break;
    default: break;
  }
  return result;
}

bool InputBackend::tap(Key key) {
  if (!keyDown(key)) return false;
  m_impl->delay();
  return keyUp(key);
}

Modifier InputBackend::activeModifiers() const { return m_impl->currentMods; }

bool InputBackend::holdModifier(Modifier mod) {
  bool ok = true;
  if (hasModifier(mod, Modifier::Shift)) ok &= keyDown(Key::ShiftLeft);
  if (hasModifier(mod, Modifier::Ctrl)) ok &= keyDown(Key::CtrlLeft);
  if (hasModifier(mod, Modifier::Alt)) ok &= keyDown(Key::AltLeft);
  if (hasModifier(mod, Modifier::Super)) ok &= keyDown(Key::SuperLeft);
  return ok;
}

bool InputBackend::releaseModifier(Modifier mod) {
  bool ok = true;
  if (hasModifier(mod, Modifier::Shift)) ok &= keyUp(Key::ShiftLeft);
  if (hasModifier(mod, Modifier::Ctrl)) ok &= keyUp(Key::CtrlLeft);
  if (hasModifier(mod, Modifier::Alt)) ok &= keyUp(Key::AltLeft);
  if (hasModifier(mod, Modifier::Super)) ok &= keyUp(Key::SuperLeft);
  return ok;
}

bool InputBackend::releaseAllModifiers() {
  return releaseModifier(Modifier::Shift | Modifier::Ctrl | Modifier::Alt | Modifier::Super);
}

bool InputBackend::combo(Modifier mods, Key key) {
  if (!holdModifier(mods)) return false;
  m_impl->delay();
  bool ok = tap(key);
  m_impl->delay();
  releaseModifier(mods);
  return ok;
}

bool InputBackend::typeText(const std::u32string& text) {
  return m_impl->typeUnicode(text);
}

bool InputBackend::typeText(const std::string& utf8Text) {
  std::u32string utf32;
  size_t i = 0;
  while (i < utf8Text.size()) {
    char32_t cp = 0;
    unsigned char c = utf8Text[i];
    if ((c & 0x80) == 0) { cp = c; i += 1; }
    else if ((c & 0xE0) == 0xC0) {
      cp = (c & 0x1F) << 6;
      if (i + 1 < utf8Text.size()) cp |= (utf8Text[i+1] & 0x3F);
      i += 2;
    }
    else if ((c & 0xF0) == 0xE0) {
      cp = (c & 0x0F) << 12;
      if (i + 1 < utf8Text.size()) cp |= (utf8Text[i+1] & 0x3F) << 6;
      if (i + 2 < utf8Text.size()) cp |= (utf8Text[i+2] & 0x3F);
      i += 3;
    }
    else if ((c & 0xF8) == 0xF0) {
      cp = (c & 0x07) << 18;
      if (i + 1 < utf8Text.size()) cp |= (utf8Text[i+1] & 0x3F) << 12;
      if (i + 2 < utf8Text.size()) cp |= (utf8Text[i+2] & 0x3F) << 6;
      if (i + 3 < utf8Text.size()) cp |= (utf8Text[i+3] & 0x3F);
      i += 4;
    }
    else { i += 1; continue; }
    utf32.push_back(cp);
  }
  return typeText(utf32);
}

bool InputBackend::typeCharacter(char32_t codepoint) {
  return typeText(std::u32string(1, codepoint));
}

void InputBackend::flush() {
  // CGEventPost is synchronous
}

void InputBackend::setKeyDelay(uint32_t delayUs) {
  m_impl->keyDelayUs = delayUs;
}

} // namespace backend

#endif // __APPLE__