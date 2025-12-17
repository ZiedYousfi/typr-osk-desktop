// input_macos.mm
// macOS Quartz backend for keyboard input injection (Objective-C++)

#include "input.hpp"

#ifdef __APPLE__

#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h> // For kVK_* virtual key codes
#import <Foundation/Foundation.h>
#include <codecvt>
#include <locale>

namespace input {

namespace {

// Convert input::Key to macOS virtual key code (CGKeyCode)
CGKeyCode keyToVirtualKeyCode(Key key) {
  switch (key) {
  case Key::A:
    return kVK_ANSI_A;
  case Key::B:
    return kVK_ANSI_B;
  case Key::C:
    return kVK_ANSI_C;
  case Key::D:
    return kVK_ANSI_D;
  case Key::E:
    return kVK_ANSI_E;
  case Key::F:
    return kVK_ANSI_F;
  case Key::G:
    return kVK_ANSI_G;
  case Key::H:
    return kVK_ANSI_H;
  case Key::I:
    return kVK_ANSI_I;
  case Key::J:
    return kVK_ANSI_J;
  case Key::K:
    return kVK_ANSI_K;
  case Key::L:
    return kVK_ANSI_L;
  case Key::M:
    return kVK_ANSI_M;
  case Key::N:
    return kVK_ANSI_N;
  case Key::O:
    return kVK_ANSI_O;
  case Key::P:
    return kVK_ANSI_P;
  case Key::Q:
    return kVK_ANSI_Q;
  case Key::R:
    return kVK_ANSI_R;
  case Key::S:
    return kVK_ANSI_S;
  case Key::T:
    return kVK_ANSI_T;
  case Key::U:
    return kVK_ANSI_U;
  case Key::V:
    return kVK_ANSI_V;
  case Key::W:
    return kVK_ANSI_W;
  case Key::X:
    return kVK_ANSI_X;
  case Key::Y:
    return kVK_ANSI_Y;
  case Key::Z:
    return kVK_ANSI_Z;
  case Key::Num0:
    return kVK_ANSI_0;
  case Key::Num1:
    return kVK_ANSI_1;
  case Key::Num2:
    return kVK_ANSI_2;
  case Key::Num3:
    return kVK_ANSI_3;
  case Key::Num4:
    return kVK_ANSI_4;
  case Key::Num5:
    return kVK_ANSI_5;
  case Key::Num6:
    return kVK_ANSI_6;
  case Key::Num7:
    return kVK_ANSI_7;
  case Key::Num8:
    return kVK_ANSI_8;
  case Key::Num9:
    return kVK_ANSI_9;
  case Key::Enter:
    return kVK_Return;
  case Key::Escape:
    return kVK_Escape;
  case Key::Backspace:
    return kVK_Delete;
  case Key::Tab:
    return kVK_Tab;
  case Key::Space:
    return kVK_Space;
  case Key::Left:
    return kVK_LeftArrow;
  case Key::Right:
    return kVK_RightArrow;
  case Key::Up:
    return kVK_UpArrow;
  case Key::Down:
    return kVK_DownArrow;
  case Key::F1:
    return kVK_F1;
  case Key::F2:
    return kVK_F2;
  case Key::F3:
    return kVK_F3;
  case Key::F4:
    return kVK_F4;
  case Key::F5:
    return kVK_F5;
  case Key::F6:
    return kVK_F6;
  case Key::F7:
    return kVK_F7;
  case Key::F8:
    return kVK_F8;
  case Key::F9:
    return kVK_F9;
  case Key::F10:
    return kVK_F10;
  case Key::F11:
    return kVK_F11;
  case Key::F12:
    return kVK_F12;
  case Key::Unknown:
  default:
    return UINT16_MAX; // Invalid key code
  }
}

// Convert input::Mod to CGEventFlags
CGEventFlags modsToCGFlags(Mod mods) {
  CGEventFlags flags = 0;
  if (mods & Mod_Shift)
    flags |= kCGEventFlagMaskShift;
  if (mods & Mod_Ctrl)
    flags |= kCGEventFlagMaskControl;
  if (mods & Mod_Alt)
    flags |= kCGEventFlagMaskAlternate;
  if (mods & Mod_Super)
    flags |= kCGEventFlagMaskCommand;
  return flags;
}

// Post a keyboard event
bool postKeyEvent(CGKeyCode keyCode, bool keyDown, CGEventFlags flags) {
  CGEventRef event = CGEventCreateKeyboardEvent(nullptr, keyCode, keyDown);
  if (!event) {
    return false;
  }

  CGEventSetFlags(event, flags);
  CGEventPost(kCGHIDEventTap, event);
  CFRelease(event);
  return true;
}

// Check if we have accessibility permissions (required for event injection)
bool checkAccessibilityPermission() {
  return AXIsProcessTrustedWithOptions(nullptr);
}

// Request accessibility permissions (shows system dialog)
bool requestAccessibilityPermission() {
  NSDictionary *options =
      @{(__bridge NSString *)kAXTrustedCheckOptionPrompt : @YES};
  return AXIsProcessTrustedWithOptions((__bridge CFDictionaryRef)options);
}

// Convert UTF-32 to UTF-16
std::u16string utf32ToUtf16(const std::u32string &text) {
  std::u16string utf16;
  utf16.reserve(text.size() * 2);

  for (char32_t codepoint : text) {
    if (codepoint <= 0xFFFF) {
      utf16.push_back(static_cast<char16_t>(codepoint));
    } else if (codepoint <= 0x10FFFF) {
      codepoint -= 0x10000;
      utf16.push_back(static_cast<char16_t>(0xD800 | (codepoint >> 10)));
      utf16.push_back(static_cast<char16_t>(0xDC00 | (codepoint & 0x3FF)));
    }
  }
  return utf16;
}

// Convert UTF-8 to UTF-32
std::u32string utf8ToUtf32(const std::string &utf8) {
  std::u32string result;
  result.reserve(utf8.size());

  size_t i = 0;
  while (i < utf8.size()) {
    char32_t codepoint = 0;
    unsigned char c = utf8[i];

    if ((c & 0x80) == 0) {
      codepoint = c;
      i += 1;
    } else if ((c & 0xE0) == 0xC0) {
      codepoint = (c & 0x1F) << 6;
      if (i + 1 < utf8.size())
        codepoint |= (utf8[i + 1] & 0x3F);
      i += 2;
    } else if ((c & 0xF0) == 0xE0) {
      codepoint = (c & 0x0F) << 12;
      if (i + 1 < utf8.size())
        codepoint |= (utf8[i + 1] & 0x3F) << 6;
      if (i + 2 < utf8.size())
        codepoint |= (utf8[i + 2] & 0x3F);
      i += 3;
    } else if ((c & 0xF8) == 0xF0) {
      codepoint = (c & 0x07) << 18;
      if (i + 1 < utf8.size())
        codepoint |= (utf8[i + 1] & 0x3F) << 12;
      if (i + 2 < utf8.size())
        codepoint |= (utf8[i + 2] & 0x3F) << 6;
      if (i + 3 < utf8.size())
        codepoint |= (utf8[i + 3] & 0x3F);
      i += 4;
    } else {
      i += 1; // Skip invalid byte
      continue;
    }

    result.push_back(codepoint);
  }
  return result;
}

} // namespace

// Platform-specific implementation structure
struct InputBackend::Impl {
  bool permissionGranted{false};

  Impl() { permissionGranted = checkAccessibilityPermission(); }

  Capabilities capabilities() const {
    Capabilities caps;
    caps.canInjectEvents = permissionGranted;
    caps.canActAsHID = false; // Quartz is soft injection only
    caps.needsAccessibilityPerm = true;
    caps.needsInputMonitoringPerm = false;
    return caps;
  }

  bool isReady() const { return permissionGranted; }

  bool requestPermissions() {
    permissionGranted = requestAccessibilityPermission();
    return permissionGranted;
  }

  bool keyDown(Key key, Mod mods) {
    CGKeyCode keyCode = keyToVirtualKeyCode(key);
    if (keyCode == UINT16_MAX) {
      return false;
    }
    return postKeyEvent(keyCode, true, modsToCGFlags(mods));
  }

  bool keyUp(Key key, Mod mods) {
    CGKeyCode keyCode = keyToVirtualKeyCode(key);
    if (keyCode == UINT16_MAX) {
      return false;
    }
    return postKeyEvent(keyCode, false, modsToCGFlags(mods));
  }

  bool tap(Key key, Mod mods) { return keyDown(key, mods) && keyUp(key, mods); }

  bool typeText(const std::u32string &text) {
    if (text.empty()) {
      return true;
    }

    std::u16string utf16 = utf32ToUtf16(text);

    // macOS CGEventKeyboardSetUnicodeString has a limit of ~20 characters
    constexpr size_t kMaxCharsPerEvent = 20;

    for (size_t i = 0; i < utf16.size(); i += kMaxCharsPerEvent) {
      size_t chunkSize = std::min(kMaxCharsPerEvent, utf16.size() - i);

      CGEventRef eventDown = CGEventCreateKeyboardEvent(nullptr, 0, true);
      if (!eventDown) {
        return false;
      }

      CGEventKeyboardSetUnicodeString(
          eventDown, static_cast<UniCharCount>(chunkSize),
          reinterpret_cast<const UniChar *>(utf16.data() + i));

      CGEventPost(kCGHIDEventTap, eventDown);
      CFRelease(eventDown);

      CGEventRef eventUp = CGEventCreateKeyboardEvent(nullptr, 0, false);
      if (!eventUp) {
        return false;
      }

      CGEventKeyboardSetUnicodeString(
          eventUp, static_cast<UniCharCount>(chunkSize),
          reinterpret_cast<const UniChar *>(utf16.data() + i));

      CGEventPost(kCGHIDEventTap, eventUp);
      CFRelease(eventUp);
    }

    return true;
  }

  bool typeText(const std::string &utf8Text) {
    return typeText(utf8ToUtf32(utf8Text));
  }
};

// InputBackend public interface implementation

InputBackend::InputBackend() : m_impl(new Impl()) {}

InputBackend::~InputBackend() { delete m_impl; }

InputBackend::InputBackend(InputBackend &&other) noexcept
    : m_impl(other.m_impl) {
  other.m_impl = nullptr;
}

InputBackend &InputBackend::operator=(InputBackend &&other) noexcept {
  if (this != &other) {
    delete m_impl;
    m_impl = other.m_impl;
    other.m_impl = nullptr;
  }
  return *this;
}

Capabilities InputBackend::capabilities() const {
  return m_impl ? m_impl->capabilities() : Capabilities{};
}

bool InputBackend::isReady() const { return m_impl && m_impl->isReady(); }

bool InputBackend::requestPermissions() {
  return m_impl && m_impl->requestPermissions();
}

bool InputBackend::keyDown(Key key, Mod mods) {
  return m_impl && m_impl->keyDown(key, mods);
}

bool InputBackend::keyUp(Key key, Mod mods) {
  return m_impl && m_impl->keyUp(key, mods);
}

bool InputBackend::tap(Key key, Mod mods) {
  return m_impl && m_impl->tap(key, mods);
}

bool InputBackend::typeText(const std::u32string &text) {
  return m_impl && m_impl->typeText(text);
}

bool InputBackend::typeText(const std::string &utf8Text) {
  return m_impl && m_impl->typeText(utf8Text);
}

} // namespace input

#endif // __APPLE__
