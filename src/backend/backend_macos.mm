// input_macos.mm

#ifdef __APPLE__
#include "backend.hpp"

#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h> // For kVK_* virtual key codes
#import <Foundation/Foundation.h>
#include <codecvt>
#include <locale>
#include <unordered_map>
#include <vector>
#include <QDebug>
#include <QString>
#include <cstdio>
#include <cstdlib>

namespace backend {

namespace {

// Translates a virtual key code and modifiers to a Unicode string using the current keyboard layout.
// This queries the system to find out what character a physical key produces.
std::u32string getCharacterForKeyCode(CGKeyCode keyCode, CGEventFlags flags) {
  TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardLayoutInputSource();
  if (!currentKeyboard) return U"";

  CFDataRef layoutData = (CFDataRef)TISGetInputSourceProperty(
      currentKeyboard, kTISPropertyUnicodeKeyLayoutData);

  if (!layoutData) {
    CFRelease(currentKeyboard);
    return U"";
  }

  const UCKeyboardLayout *keyboardLayout =
      (const UCKeyboardLayout *)CFDataGetBytePtr(layoutData);

  UInt32 deadKeyState = 0;
  UniChar unicodeString[4];
  UniCharCount actualStringLength;

  // Convert CGEventFlags to Carbon modifiers
  UInt32 modifiers = 0;
  if (flags & kCGEventFlagMaskShift) modifiers |= shiftKey >> 8;
  if (flags & kCGEventFlagMaskAlphaShift) modifiers |= alphaLock >> 8;
  if (flags & kCGEventFlagMaskAlternate) modifiers |= optionKey >> 8;
  if (flags & kCGEventFlagMaskControl) modifiers |= controlKey >> 8;
  if (flags & kCGEventFlagMaskCommand) modifiers |= cmdKey >> 8;

  OSStatus status = UCKeyTranslate(
      keyboardLayout,
      keyCode,
      kUCKeyActionDown,
      modifiers,
      LMGetKbdType(),
      kUCKeyTranslateNoDeadKeysBit,
      &deadKeyState,
      4,
      &actualStringLength,
      unicodeString
  );

  CFRelease(currentKeyboard);

  if (status == noErr && actualStringLength > 0) {
    std::u32string result;
    for (UniCharCount i = 0; i < actualStringLength; ++i) {
      result += (char32_t)unicodeString[i];
    }
    return result;
  }

  return U"";
}

// Debug helper utilities
// Debugging helper: enable backend debug logs with env var TYPR_OSK_DEBUG_BACKEND
static bool debugBackendEnabled() {
  static bool enabled = []() {
    const char *env = std::getenv("TYPR_OSK_DEBUG_BACKEND");
    return env && env[0] != '\0';
  }();
  return enabled;
}

static QString modsToString(Mod mods) {
  QString s;
  if (mods & Mod_Shift) s += "Shift|";
  if (mods & Mod_Ctrl) s += "Ctrl|";
  if (mods & Mod_Alt) s += "Alt|";
  if (mods & Mod_Super) s += "Super|";
  if (s.isEmpty()) return QStringLiteral("None");
  s.chop(1);
  return s;
}

// Small helper for debug logging: produce a printable (ASCII/escaped) preview
static std::string debugPreviewFromUtf32(const std::u32string &s) {
  std::string out;
  for (char32_t cp : s) {
    if (cp <= 0x7F && cp >= 0x20) {
      out.push_back(static_cast<char>(cp));
    } else if (cp == U'\n') {
      out += "\\n";
    } else {
      char buf[16];
      snprintf(buf, sizeof(buf), "\\u%04x", (unsigned)cp);
      out += buf;
    }
    if (out.size() > 64) { out += "..."; break; }
  }
  if (out.empty()) out = "";
  return out;
}

// logTypeText removed - use unconditional logging elsewhere

// Helper to normalize characters for comparison (lowercase, no modifiers)
char32_t normalizeChar(char32_t c) {
  if (c >= U'A' && c <= U'Z') {
    return c - U'A' + U'a';
  }
  return c;
}

// Build a mapping from our Key enum to macOS virtual key codes by scanning the keyboard layout
class KeyboardLayoutMapper {
public:
  KeyboardLayoutMapper() {
    buildMappings();
  }

  CGKeyCode getKeyCode(Key key) const {
    // For non-character keys, return fixed mappings
    auto it = m_fixedMappings.find(key);
    if (it != m_fixedMappings.end()) {
      return it->second;
    }

    // For character keys, use the dynamically built mapping
    auto it2 = m_keyToVirtualKey.find(key);
    if (it2 != m_keyToVirtualKey.end()) {
      return it2->second;
    }

    return UINT16_MAX;
  }

private:
  void buildMappings() {
    // First, set up fixed mappings for non-character keys (these don't change with layout)
    m_fixedMappings[Key::Enter] = kVK_Return;
    m_fixedMappings[Key::Escape] = kVK_Escape;
    m_fixedMappings[Key::Backspace] = kVK_Delete;
    m_fixedMappings[Key::Tab] = kVK_Tab;
    m_fixedMappings[Key::Space] = kVK_Space;
    m_fixedMappings[Key::Delete] = kVK_ForwardDelete;
    m_fixedMappings[Key::Insert] = kVK_Help;
    m_fixedMappings[Key::Left] = kVK_LeftArrow;
    m_fixedMappings[Key::Right] = kVK_RightArrow;
    m_fixedMappings[Key::Up] = kVK_UpArrow;
    m_fixedMappings[Key::Down] = kVK_DownArrow;
    m_fixedMappings[Key::Home] = kVK_Home;
    m_fixedMappings[Key::End] = kVK_End;
    m_fixedMappings[Key::PageUp] = kVK_PageUp;
    m_fixedMappings[Key::PageDown] = kVK_PageDown;
    m_fixedMappings[Key::F1] = kVK_F1;
    m_fixedMappings[Key::F2] = kVK_F2;
    m_fixedMappings[Key::F3] = kVK_F3;
    m_fixedMappings[Key::F4] = kVK_F4;
    m_fixedMappings[Key::F5] = kVK_F5;
    m_fixedMappings[Key::F6] = kVK_F6;
    m_fixedMappings[Key::F7] = kVK_F7;
    m_fixedMappings[Key::F8] = kVK_F8;
    m_fixedMappings[Key::F9] = kVK_F9;
    m_fixedMappings[Key::F10] = kVK_F10;
    m_fixedMappings[Key::F11] = kVK_F11;
    m_fixedMappings[Key::F12] = kVK_F12;
    m_fixedMappings[Key::F13] = kVK_F13;
    m_fixedMappings[Key::F14] = kVK_F14;
    m_fixedMappings[Key::F15] = kVK_F15;
    m_fixedMappings[Key::F16] = kVK_F16;
    m_fixedMappings[Key::F17] = kVK_F17;
    m_fixedMappings[Key::F18] = kVK_F18;
    m_fixedMappings[Key::F19] = kVK_F19;
    m_fixedMappings[Key::F20] = kVK_F20;
    m_fixedMappings[Key::ShiftLeft] = kVK_Shift;
    m_fixedMappings[Key::ShiftRight] = kVK_RightShift;
    m_fixedMappings[Key::CtrlLeft] = kVK_Control;
    m_fixedMappings[Key::CtrlRight] = kVK_RightControl;
    m_fixedMappings[Key::AltLeft] = kVK_Option;
    m_fixedMappings[Key::AltRight] = kVK_RightOption;
    m_fixedMappings[Key::SuperLeft] = kVK_Command;
    m_fixedMappings[Key::SuperRight] = kVK_RightCommand;
    m_fixedMappings[Key::CapsLock] = kVK_CapsLock;
    m_fixedMappings[Key::NumLock] = kVK_ANSI_KeypadClear;
    m_fixedMappings[Key::PrintScreen] = kVK_F13;
    m_fixedMappings[Key::ScrollLock] = kVK_ANSI_KeypadClear;
    m_fixedMappings[Key::Pause] = kVK_F15;
    m_fixedMappings[Key::Menu] = kVK_F4;
    m_fixedMappings[Key::Mute] = kVK_Mute;
    m_fixedMappings[Key::VolumeDown] = kVK_VolumeDown;
    m_fixedMappings[Key::VolumeUp] = kVK_VolumeUp;
    m_fixedMappings[Key::MediaPlayPause] = kVK_F8;
    m_fixedMappings[Key::MediaNext] = kVK_F9;
    m_fixedMappings[Key::MediaPrevious] = kVK_F7;
    m_fixedMappings[Key::MediaStop] = kVK_F8;
    m_fixedMappings[Key::Numpad0] = kVK_ANSI_Keypad0;
    m_fixedMappings[Key::Numpad1] = kVK_ANSI_Keypad1;
    m_fixedMappings[Key::Numpad2] = kVK_ANSI_Keypad2;
    m_fixedMappings[Key::Numpad3] = kVK_ANSI_Keypad3;
    m_fixedMappings[Key::Numpad4] = kVK_ANSI_Keypad4;
    m_fixedMappings[Key::Numpad5] = kVK_ANSI_Keypad5;
    m_fixedMappings[Key::Numpad6] = kVK_ANSI_Keypad6;
    m_fixedMappings[Key::Numpad7] = kVK_ANSI_Keypad7;
    m_fixedMappings[Key::Numpad8] = kVK_ANSI_Keypad8;
    m_fixedMappings[Key::Numpad9] = kVK_ANSI_Keypad9;
    m_fixedMappings[Key::NumpadDivide] = kVK_ANSI_KeypadDivide;
    m_fixedMappings[Key::NumpadMultiply] = kVK_ANSI_KeypadMultiply;
    m_fixedMappings[Key::NumpadMinus] = kVK_ANSI_KeypadMinus;
    m_fixedMappings[Key::NumpadPlus] = kVK_ANSI_KeypadPlus;
    m_fixedMappings[Key::NumpadEnter] = kVK_ANSI_KeypadEnter;
    m_fixedMappings[Key::NumpadDecimal] = kVK_ANSI_KeypadDecimal;

    // Now scan the keyboard layout to find which physical keys produce which characters
    // We'll scan all possible virtual key codes (0-127 is the typical range)
    for (CGKeyCode vk = 0; vk < 128; ++vk) {
      // Get the character this key produces without modifiers
      std::u32string chars = getCharacterForKeyCode(vk, 0);
      if (chars.empty()) continue;

      char32_t c = normalizeChar(chars[0]);

      // Map letters A-Z
      if (c >= U'a' && c <= U'z') {
        Key key = static_cast<Key>(static_cast<uint16_t>(Key::A) + (c - U'a'));
        if (m_keyToVirtualKey.find(key) == m_keyToVirtualKey.end()) {
          m_keyToVirtualKey[key] = vk;
        }
      }
      // Map numbers 0-9
      else if (c >= U'0' && c <= U'9') {
        Key key = static_cast<Key>(static_cast<uint16_t>(Key::Num0) + (c - U'0'));
        if (m_keyToVirtualKey.find(key) == m_keyToVirtualKey.end()) {
          m_keyToVirtualKey[key] = vk;
        }
      }
      // Map punctuation and special characters
      else {
        Key key = Key::Unknown;
        switch (c) {
          case U'`': key = Key::Grave; break;
          case U'-': key = Key::Minus; break;
          case U'=': key = Key::Equal; break;
          case U'[': key = Key::LeftBracket; break;
          case U']': key = Key::RightBracket; break;
          case U'\\': key = Key::Backslash; break;
          case U';': key = Key::Semicolon; break;
          case U'\'': key = Key::Apostrophe; break;
          case U',': key = Key::Comma; break;
          case U'.': key = Key::Period; break;
          case U'/': key = Key::Slash; break;
          default: break;
        }
        if (key != Key::Unknown && m_keyToVirtualKey.find(key) == m_keyToVirtualKey.end()) {
          m_keyToVirtualKey[key] = vk;
        }
      }
    }
  }

  std::unordered_map<Key, CGKeyCode> m_fixedMappings;
  std::unordered_map<Key, CGKeyCode> m_keyToVirtualKey;
};

// Check if a key should be translated to a character instead of just sending the virtual key code.
bool shouldTranslateToCharacter(Key key) {
  switch (key) {
  case Key::A: case Key::B: case Key::C: case Key::D: case Key::E: case Key::F:
  case Key::G: case Key::H: case Key::I: case Key::J: case Key::K: case Key::L:
  case Key::M: case Key::N: case Key::O: case Key::P: case Key::Q: case Key::R:
  case Key::S: case Key::T: case Key::U: case Key::V: case Key::W: case Key::X:
  case Key::Y: case Key::Z:
  case Key::Num0: case Key::Num1: case Key::Num2: case Key::Num3: case Key::Num4:
  case Key::Num5: case Key::Num6: case Key::Num7: case Key::Num8: case Key::Num9:
  case Key::Grave: case Key::Minus: case Key::Equal: case Key::LeftBracket:
  case Key::RightBracket: case Key::Backslash: case Key::Semicolon:
  case Key::Apostrophe: case Key::Comma: case Key::Period: case Key::Slash:
    return true;
  default:
    return false;
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
  KeyboardLayoutMapper layoutMapper;

  Impl() {
    permissionGranted = checkAccessibilityPermission();
    if (debugBackendEnabled()) {
      qDebug() << "[backend::macos] permissionGranted:" << permissionGranted;
    }
  }

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
    CGKeyCode keyCode = layoutMapper.getKeyCode(key);
    qDebug() << "[backend::macos] keyDown(start):"
             << QString::fromStdString(keyToString(key))
             << "mappedVK:" << (keyCode == UINT16_MAX ? QStringLiteral("<none>") : QString::number(keyCode))
             << "mods:" << modsToString(mods);
    if (keyCode == UINT16_MAX) {
      qDebug() << "[backend::macos] keyDown: aborting (no mapping for)"
               << QString::fromStdString(keyToString(key));
      return false;
    }

    // Helper to attempt a left-preferring modifier keycode (fallback to right)
    auto getModVK = [&](Key leftKey, Key rightKey) -> CGKeyCode {
      CGKeyCode mk = layoutMapper.getKeyCode(leftKey);
      if (mk != UINT16_MAX) return mk;
      return layoutMapper.getKeyCode(rightKey);
    };

    bool ok = true;

    // Press modifiers physically (Shift, Ctrl, Alt, Super)
    if (mods & Mod_Shift) {
      CGKeyCode mk = getModVK(Key::ShiftLeft, Key::ShiftRight);
      if (mk != UINT16_MAX) {
        bool r = postKeyEvent(mk, true, 0);
        qDebug() << "[backend::macos] keyDown: press Shift vk:" << mk << "result:" << r;
        ok = r && ok;
      } else {
        qDebug() << "[backend::macos] keyDown: Shift mapping not found";
      }
    }
    if (mods & Mod_Ctrl) {
      CGKeyCode mk = getModVK(Key::CtrlLeft, Key::CtrlRight);
      if (mk != UINT16_MAX) {
        bool r = postKeyEvent(mk, true, 0);
        qDebug() << "[backend::macos] keyDown: press Ctrl vk:" << mk << "result:" << r;
        ok = r && ok;
      } else {
        qDebug() << "[backend::macos] keyDown: Ctrl mapping not found";
      }
    }
    if (mods & Mod_Alt) {
      CGKeyCode mk = getModVK(Key::AltLeft, Key::AltRight);
      if (mk != UINT16_MAX) {
        bool r = postKeyEvent(mk, true, 0);
        qDebug() << "[backend::macos] keyDown: press Alt vk:" << mk << "result:" << r;
        ok = r && ok;
      } else {
        qDebug() << "[backend::macos] keyDown: Alt mapping not found";
      }
    }
    if (mods & Mod_Super) {
      CGKeyCode mk = getModVK(Key::SuperLeft, Key::SuperRight);
      if (mk != UINT16_MAX) {
        bool r = postKeyEvent(mk, true, 0);
        qDebug() << "[backend::macos] keyDown: press Super vk:" << mk << "result:" << r;
        ok = r && ok;
      } else {
        qDebug() << "[backend::macos] keyDown: Super mapping not found";
      }
    }

    qDebug() << "[backend::macos] keyDown: pressing main key"
             << QString::fromStdString(keyToString(key))
             << "keyCode:" << keyCode
             << "mods:" << modsToString(mods);

    bool r_main = postKeyEvent(keyCode, true, 0);
    qDebug() << "[backend::macos] keyDown: main key post result:" << r_main;
    ok = r_main && ok;
    qDebug() << "[backend::macos] keyDown: overall result:" << ok;
    return ok;
  }

  bool keyUp(Key key, Mod mods) {
    CGKeyCode keyCode = layoutMapper.getKeyCode(key);
    qDebug() << "[backend::macos] keyUp(start):"
             << QString::fromStdString(keyToString(key))
             << "mappedVK:" << (keyCode == UINT16_MAX ? QStringLiteral("<none>") : QString::number(keyCode))
             << "mods:" << modsToString(mods);
    if (keyCode == UINT16_MAX) {
      qDebug() << "[backend::macos] keyUp: aborting (no mapping for)"
               << QString::fromStdString(keyToString(key));
      return false;
    }

    // Helper to attempt a left-preferring modifier keycode (fallback to right)
    auto getModVK = [&](Key leftKey, Key rightKey) -> CGKeyCode {
      CGKeyCode mk = layoutMapper.getKeyCode(leftKey);
      if (mk != UINT16_MAX) return mk;
      return layoutMapper.getKeyCode(rightKey);
    };

    bool ok = true;

    // Release main key first
    ok = postKeyEvent(keyCode, false, 0) && ok;

    // Release modifiers (Shift, Ctrl, Alt, Super)
    if (mods & Mod_Shift) {
      CGKeyCode mk = getModVK(Key::ShiftLeft, Key::ShiftRight);
      if (mk != UINT16_MAX) {
        if (debugBackendEnabled()) {
          qDebug() << "[backend::macos] keyUp: releasing Shift vk:" << mk;
        }
        ok = postKeyEvent(mk, false, 0) && ok;
      }
    }
    if (mods & Mod_Ctrl) {
      CGKeyCode mk = getModVK(Key::CtrlLeft, Key::CtrlRight);
      if (mk != UINT16_MAX) {
        if (debugBackendEnabled()) {
          qDebug() << "[backend::macos] keyUp: releasing Ctrl vk:" << mk;
        }
        ok = postKeyEvent(mk, false, 0) && ok;
      }
    }
    if (mods & Mod_Alt) {
      CGKeyCode mk = getModVK(Key::AltLeft, Key::AltRight);
      if (mk != UINT16_MAX) {
        if (debugBackendEnabled()) {
          qDebug() << "[backend::macos] keyUp: releasing Alt vk:" << mk;
        }
        ok = postKeyEvent(mk, false, 0) && ok;
      }
    }
    if (mods & Mod_Super) {
      CGKeyCode mk = getModVK(Key::SuperLeft, Key::SuperRight);
      if (mk != UINT16_MAX) {
        if (debugBackendEnabled()) {
          qDebug() << "[backend::macos] keyUp: releasing Super vk:" << mk;
        }
        ok = postKeyEvent(mk, false, 0) && ok;
      }
    }

    if (debugBackendEnabled()) {
      qDebug() << "[backend::macos] keyUp: main key"
               << QString::fromStdString(keyToString(key))
               << "keyCode:" << keyCode
               << "mods:" << modsToString(mods);
    }

    return ok;
  }

  bool tap(Key key, Mod mods) {
    CGKeyCode vk = layoutMapper.getKeyCode(key);
    if (vk == UINT16_MAX) {
      if (debugBackendEnabled()) {
        qDebug() << "[backend::macos] tap: no mapping for"
                 << QString::fromStdString(keyToString(key));
      }
      return false;
    }

    if (debugBackendEnabled()) {
      qDebug() << "[backend::macos] tap:" << QString::fromStdString(keyToString(key))
               << "keyCode:" << vk << "mods:" << modsToString(mods);
    }

    // Use UCKeyboardTranslate to determine what character should be produced
    // with the current keyboard layout and modifiers.
    // We only use translation for simple character input (no Command/Control)
    // to preserve keyboard shortcuts, but allow Alt/Option for special characters.
    bool hasShortcutMods = (mods & (Mod_Ctrl | Mod_Super));
    if (shouldTranslateToCharacter(key) && !hasShortcutMods) {
      std::u32string translated = getCharacterForKeyCode(vk, modsToCGFlags(mods));
      if (!translated.empty()) {
        if (debugBackendEnabled()) {
          qDebug() << "[backend::macos] tap: translating to Unicode, length:"
                   << (int)translated.size();
        }
        return typeText(translated);
      }
    }

    // If no character translation is available or appropriate, fallback to virtual key codes
    return keyDown(key, mods) && keyUp(key, mods);
  }

  // Handle KeyStroke with optional character data
  bool keyDown(const KeyStroke &keystroke) {
    if (keystroke.character && keystroke.character->size() > 0) {
      // Prefer sending a physical key when possible so that keyDown/keyUp
      // semantics (hold/release) are preserved. If the key doesn't map to a
      // physical key in the current layout, fall back to direct Unicode input.
      CGKeyCode vk = layoutMapper.getKeyCode(keystroke.key);
      if (vk != UINT16_MAX) {
        return keyDown(keystroke.key, keystroke.mods);
      }
      return typeText(*keystroke.character);
    }
    return keyDown(keystroke.key, keystroke.mods);
  }

  bool keyUp(const KeyStroke &keystroke) {
    if (keystroke.character && keystroke.character->size() > 0) {
      CGKeyCode vk = layoutMapper.getKeyCode(keystroke.key);
      if (vk != UINT16_MAX) {
        if (debugBackendEnabled()) {
          qDebug() << "[backend::macos] keyUp(keystroke): releasing physical key"
                   << QString::fromStdString(keyToString(keystroke.key))
                   << "keyCode:" << vk << "mods:" << modsToString(keystroke.mods);
        }
        return keyUp(keystroke.key, keystroke.mods);
      }
      if (debugBackendEnabled()) {
        qDebug() << "[backend::macos] keyUp(keystroke): nothing to release for Unicode input";
      }
      return true;
    }
    if (debugBackendEnabled()) {
      qDebug() << "[backend::macos] keyUp(keystroke): releasing physical key"
               << QString::fromStdString(keyToString(keystroke.key)) << "mods:" << modsToString(keystroke.mods);
    }
    return keyUp(keystroke.key, keystroke.mods);
  }

  bool tap(const KeyStroke &keystroke) {
    if (keystroke.character && keystroke.character->size() > 0) {
      CGKeyCode vk = layoutMapper.getKeyCode(keystroke.key);
      if (vk != UINT16_MAX) {
        if (debugBackendEnabled()) {
          qDebug() << "[backend::macos] tap(keystroke): physical tap for"
                   << QString::fromStdString(keyToString(keystroke.key)) << "keyCode:" << vk << "mods:" << modsToString(keystroke.mods);
        }
        return tap(keystroke.key, keystroke.mods);
      }
      if (debugBackendEnabled()) {
        qDebug() << "[backend::macos] tap(keystroke): typing Unicode text length"
                 << (int)keystroke.character->size();
      }
      return typeText(*keystroke.character);
    }
    if (debugBackendEnabled()) {
      qDebug() << "[backend::macos] tap(keystroke): physical tap for"
               << QString::fromStdString(keyToString(keystroke.key)) << "mods:" << modsToString(keystroke.mods);
    }
    return tap(keystroke.key, keystroke.mods);
  }

  bool typeText(const std::u32string &text) {
    if (text.empty()) {
      qDebug() << "[backend::macos] typeText: empty text";
      return true;
    }
    qDebug() << "[backend::macos] typeText:start length=" << (int)text.size()
             << "preview=" << QString::fromStdString(debugPreviewFromUtf32(text));

    std::u16string utf16 = utf32ToUtf16(text);

    // macOS CGEventKeyboardSetUnicodeString has a limit of ~20 characters
    constexpr size_t kMaxCharsPerEvent = 20;

    for (size_t i = 0; i < utf16.size(); i += kMaxCharsPerEvent) {
      size_t chunkSize = std::min(kMaxCharsPerEvent, utf16.size() - i);

      CGEventRef eventDown = CGEventCreateKeyboardEvent(nullptr, 0, true);
      if (!eventDown) {
        qDebug() << "[backend::macos] typeText: failed to create eventDown for chunk start:" << (int)i;
        return false;
      }

      CGEventKeyboardSetUnicodeString(
          eventDown, static_cast<UniCharCount>(chunkSize),
          reinterpret_cast<const UniChar *>(utf16.data() + i));

      CGEventPost(kCGHIDEventTap, eventDown);
      qDebug() << "[backend::macos] typeText: posted eventDown chunk start:" << (int)i << "chunkSize:" << (int)chunkSize;
      CFRelease(eventDown);

      CGEventRef eventUp = CGEventCreateKeyboardEvent(nullptr, 0, false);
      if (!eventUp) {
        qDebug() << "[backend::macos] typeText: failed to create eventUp for chunk start:" << (int)i;
        return false;
      }

      CGEventKeyboardSetUnicodeString(
          eventUp, static_cast<UniCharCount>(chunkSize),
          reinterpret_cast<const UniChar *>(utf16.data() + i));

      CGEventPost(kCGHIDEventTap, eventUp);
      qDebug() << "[backend::macos] typeText: posted eventUp chunk start:" << (int)i << "chunkSize:" << (int)chunkSize;
      CFRelease(eventUp);
    }

    qDebug() << "[backend::macos] typeText: finished successfully";
    return true;
  }

  bool typeText(const std::string &utf8Text) {
    std::u32string utf32 = utf8ToUtf32(utf8Text);
    if (debugBackendEnabled()) {
      std::string preview = debugPreviewFromUtf32(utf32);
      qDebug() << "[backend::macos] typeText(utf8): length=" << (int)utf32.size()
               << "preview=" << QString::fromStdString(preview);
    }
    return typeText(utf32);
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

} // namespace backend

#endif // __APPLE__
