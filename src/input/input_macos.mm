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

// Check if a key should be typed as a Unicode character instead of using virtual key codes
// This ensures keyboard layout independence for printable characters
bool shouldTypeAsCharacter(Key key) {
  switch (key) {
  // Letters should be typed as characters to respect keyboard layout
  case Key::A:
  case Key::B:
  case Key::C:
  case Key::D:
  case Key::E:
  case Key::F:
  case Key::G:
  case Key::H:
  case Key::I:
  case Key::J:
  case Key::K:
  case Key::L:
  case Key::M:
  case Key::N:
  case Key::O:
  case Key::P:
  case Key::Q:
  case Key::R:
  case Key::S:
  case Key::T:
  case Key::U:
  case Key::V:
  case Key::W:
  case Key::X:
  case Key::Y:
  case Key::Z:
  // Numbers should be typed as characters to respect keyboard layout
  case Key::Num0:
  case Key::Num1:
  case Key::Num2:
  case Key::Num3:
  case Key::Num4:
  case Key::Num5:
  case Key::Num6:
  case Key::Num7:
  case Key::Num8:
  case Key::Num9:
  // Punctuation should be typed as characters to respect keyboard layout
  case Key::Grave:
  case Key::Minus:
  case Key::Equal:
  case Key::LeftBracket:
  case Key::RightBracket:
  case Key::Backslash:
  case Key::Semicolon:
  case Key::Apostrophe:
  case Key::Comma:
  case Key::Period:
  case Key::Slash:
    return true;
  default:
    return false;
  }
}

// Convert Key enum to the character it should produce (lowercase, no modifiers)
char32_t keyToCharacter(Key key) {
  switch (key) {
  case Key::A: return U'a';
  case Key::B: return U'b';
  case Key::C: return U'c';
  case Key::D: return U'd';
  case Key::E: return U'e';
  case Key::F: return U'f';
  case Key::G: return U'g';
  case Key::H: return U'h';
  case Key::I: return U'i';
  case Key::J: return U'j';
  case Key::K: return U'k';
  case Key::L: return U'l';
  case Key::M: return U'm';
  case Key::N: return U'n';
  case Key::O: return U'o';
  case Key::P: return U'p';
  case Key::Q: return U'q';
  case Key::R: return U'r';
  case Key::S: return U's';
  case Key::T: return U't';
  case Key::U: return U'u';
  case Key::V: return U'v';
  case Key::W: return U'w';
  case Key::X: return U'x';
  case Key::Y: return U'y';
  case Key::Z: return U'z';
  case Key::Num0: return U'0';
  case Key::Num1: return U'1';
  case Key::Num2: return U'2';
  case Key::Num3: return U'3';
  case Key::Num4: return U'4';
  case Key::Num5: return U'5';
  case Key::Num6: return U'6';
  case Key::Num7: return U'7';
  case Key::Num8: return U'8';
  case Key::Num9: return U'9';
  case Key::Grave: return U'`';
  case Key::Minus: return U'-';
  case Key::Equal: return U'=';
  case Key::LeftBracket: return U'[';
  case Key::RightBracket: return U']';
  case Key::Backslash: return U'\\';
  case Key::Semicolon: return U';';
  case Key::Apostrophe: return U'\'';
  case Key::Comma: return U',';
  case Key::Period: return U'.';
  case Key::Slash: return U'/';
  case Key::Space: return U' ';
  default: return 0;
  }
}

// Apply modifiers to a character (e.g., Shift makes it uppercase)
char32_t applyModifiersToCharacter(char32_t c, Mod mods) {
  // Handle Shift modifier for letters
  if (mods & Mod_Shift) {
    if (c >= U'a' && c <= U'z') {
      return c - U'a' + U'A';
    }
    // Handle shift for punctuation and numbers
    switch (c) {
    case U'`': return U'~';
    case U'1': return U'!';
    case U'2': return U'@';
    case U'3': return U'#';
    case U'4': return U'$';
    case U'5': return U'%';
    case U'6': return U'^';
    case U'7': return U'&';
    case U'8': return U'*';
    case U'9': return U'(';
    case U'0': return U')';
    case U'-': return U'_';
    case U'=': return U'+';
    case U'[': return U'{';
    case U']': return U'}';
    case U'\\': return U'|';
    case U';': return U':';
    case U'\'': return U'"';
    case U',': return U'<';
    case U'.': return U'>';
    case U'/': return U'?';
    default: break;
    }
  }
  return c;
}

// Convert input::Key to macOS virtual key code (CGKeyCode)
CGKeyCode keyToVirtualKeyCode(Key key) {
  switch (key) {
  // Letters
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
  // Numbers
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
  // Punctuation
  case Key::Grave:
    return kVK_ANSI_Grave;
  case Key::Minus:
    return kVK_ANSI_Minus;
  case Key::Equal:
    return kVK_ANSI_Equal;
  case Key::LeftBracket:
    return kVK_ANSI_LeftBracket;
  case Key::RightBracket:
    return kVK_ANSI_RightBracket;
  case Key::Backslash:
    return kVK_ANSI_Backslash;
  case Key::Semicolon:
    return kVK_ANSI_Semicolon;
  case Key::Apostrophe:
    return kVK_ANSI_Quote;
  case Key::Comma:
    return kVK_ANSI_Comma;
  case Key::Period:
    return kVK_ANSI_Period;
  case Key::Slash:
    return kVK_ANSI_Slash;
  // Numpad
  case Key::Numpad0:
    return kVK_ANSI_Keypad0;
  case Key::Numpad1:
    return kVK_ANSI_Keypad1;
  case Key::Numpad2:
    return kVK_ANSI_Keypad2;
  case Key::Numpad3:
    return kVK_ANSI_Keypad3;
  case Key::Numpad4:
    return kVK_ANSI_Keypad4;
  case Key::Numpad5:
    return kVK_ANSI_Keypad5;
  case Key::Numpad6:
    return kVK_ANSI_Keypad6;
  case Key::Numpad7:
    return kVK_ANSI_Keypad7;
  case Key::Numpad8:
    return kVK_ANSI_Keypad8;
  case Key::Numpad9:
    return kVK_ANSI_Keypad9;
  case Key::NumpadDivide:
    return kVK_ANSI_KeypadDivide;
  case Key::NumpadMultiply:
    return kVK_ANSI_KeypadMultiply;
  case Key::NumpadMinus:
    return kVK_ANSI_KeypadMinus;
  case Key::NumpadPlus:
    return kVK_ANSI_KeypadPlus;
  case Key::NumpadEnter:
    return kVK_ANSI_KeypadEnter;
  case Key::NumpadDecimal:
    return kVK_ANSI_KeypadDecimal;
  // Control keys
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
  case Key::Delete:
    return kVK_ForwardDelete;
  case Key::Insert:
    return kVK_Help;
  // Navigation
  case Key::Left:
    return kVK_LeftArrow;
  case Key::Right:
    return kVK_RightArrow;
  case Key::Up:
    return kVK_UpArrow;
  case Key::Down:
    return kVK_DownArrow;
  case Key::Home:
    return kVK_Home;
  case Key::End:
    return kVK_End;
  case Key::PageUp:
    return kVK_PageUp;
  case Key::PageDown:
    return kVK_PageDown;
  // Function keys
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
  case Key::F13:
    return kVK_F13;
  case Key::F14:
    return kVK_F14;
  case Key::F15:
    return kVK_F15;
  case Key::F16:
    return kVK_F16;
  case Key::F17:
    return kVK_F17;
  case Key::F18:
    return kVK_F18;
  case Key::F19:
    return kVK_F19;
  case Key::F20:
    return kVK_F20;
  // Modifiers as keys
  case Key::ShiftLeft:
    return kVK_Shift;
  case Key::ShiftRight:
    return kVK_RightShift;
  case Key::CtrlLeft:
    return kVK_Control;
  case Key::CtrlRight:
    return kVK_RightControl;
  case Key::AltLeft:
    return kVK_Option;
  case Key::AltRight:
    return kVK_RightOption;
  case Key::SuperLeft:
    return kVK_Command;
  case Key::SuperRight:
    return kVK_RightCommand;
  // Lock keys
  case Key::CapsLock:
   return kVK_CapsLock;
 case Key::NumLock:
   return kVK_ANSI_KeypadClear;
 // Special keys
 case Key::PrintScreen:
   return kVK_F13;
 case Key::ScrollLock:
   return kVK_ANSI_KeypadClear;
 case Key::Pause:
   return kVK_F15;
 case Key::Menu:
   return kVK_F4;
 case Key::Mute:
   return kVK_Mute;
 case Key::VolumeDown:
   return kVK_VolumeDown;
 case Key::VolumeUp:
   return kVK_VolumeUp;
 case Key::MediaPlayPause:
   return kVK_F8;
 case Key::MediaNext:
   return kVK_F9;
 case Key::MediaPrevious:
   return kVK_F7;
 case Key::MediaStop:
   return kVK_F8;
 case Key::CharacterInput:
 case Key::BackspaceDelete:
 case Key::Unknown:
 default:
   return UINT16_MAX;
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
    // For printable characters, use Unicode input to respect keyboard layout
    if (shouldTypeAsCharacter(key)) {
      // For keyDown, we can't really do a "half type", so we'll use virtual key code
      // but this is mainly for toggle modes. Normal typing should use tap().
      CGKeyCode keyCode = keyToVirtualKeyCode(key);
      if (keyCode == UINT16_MAX) {
        return false;
      }
      return postKeyEvent(keyCode, true, modsToCGFlags(mods));
    }

    CGKeyCode keyCode = keyToVirtualKeyCode(key);
    if (keyCode == UINT16_MAX) {
      return false;
    }
    return postKeyEvent(keyCode, true, modsToCGFlags(mods));
  }

  bool keyUp(Key key, Mod mods) {
    // For printable characters, use Unicode input to respect keyboard layout
    if (shouldTypeAsCharacter(key)) {
      // For keyUp, we can't really do a "half type", so we'll use virtual key code
      CGKeyCode keyCode = keyToVirtualKeyCode(key);
      if (keyCode == UINT16_MAX) {
        return false;
      }
      return postKeyEvent(keyCode, false, modsToCGFlags(mods));
    }

    CGKeyCode keyCode = keyToVirtualKeyCode(key);
    if (keyCode == UINT16_MAX) {
      return false;
    }
    return postKeyEvent(keyCode, false, modsToCGFlags(mods));
  }

  bool tap(Key key, Mod mods) {
    // For printable characters, use Unicode input to respect keyboard layout
    if (shouldTypeAsCharacter(key)) {
      char32_t c = keyToCharacter(key);
      if (c == 0) {
        return false;
      }
      // Apply modifiers (like Shift) to the character
      c = applyModifiersToCharacter(c, mods);
      return typeCharacter(c);
    }

    return keyDown(key, mods) && keyUp(key, mods);
  }

  // Handle KeyStroke with optional character data
  bool keyDown(const KeyStroke &keystroke) {
    if (keystroke.character && keystroke.character->size() > 0) {
      // Type the character(s) instead of pressing a key
      return typeText(*keystroke.character);
    }
    return keyDown(keystroke.key, keystroke.mods);
  }

  bool keyUp(const KeyStroke &keystroke) {
    if (keystroke.character && keystroke.character->size() > 0) {
      // For character input, we don't have a keyUp
      return true;
    }
    return keyUp(keystroke.key, keystroke.mods);
  }

  bool tap(const KeyStroke &keystroke) {
    if (keystroke.character && keystroke.character->size() > 0) {
      // For character input, just type it
      return typeText(*keystroke.character);
    }
    return tap(keystroke.key, keystroke.mods);
  }

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

  bool typeCharacter(char32_t codepoint) {
    std::u32string text(1, codepoint);
    return typeText(text);
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

bool InputBackend::keyDown(const KeyStroke &keystroke) {
  return m_impl && m_impl->keyDown(keystroke);
}

bool InputBackend::keyUp(const KeyStroke &keystroke) {
  return m_impl && m_impl->keyUp(keystroke);
}

bool InputBackend::keyDown(Key key, Mod mods) {
  return m_impl && m_impl->keyDown(key, mods);
}

bool InputBackend::keyUp(Key key, Mod mods) {
  return m_impl && m_impl->keyUp(key, mods);
}

bool InputBackend::tap(const KeyStroke &keystroke) {
  return m_impl && m_impl->tap(keystroke);
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

bool InputBackend::typeCharacter(char32_t codepoint) {
  return m_impl && m_impl->typeCharacter(codepoint);
}

} // namespace input

#endif // __APPLE__
