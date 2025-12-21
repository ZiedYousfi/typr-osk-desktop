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

  // Send a physical key down event for the given Key
  bool keyDownKey(Key key) {
    CGKeyCode keyCode = layoutMapper.getKeyCode(key);
    if (debugBackendEnabled()) {
      qDebug() << "[backend::macos] keyDownKey:" << QString::fromStdString(keyToString(key))
               << "keyCode:" << (keyCode == UINT16_MAX ? QStringLiteral("<none>") : QString::number(keyCode));
    }
    if (keyCode == UINT16_MAX) return false;
    return postKeyEvent(keyCode, true, 0);
  }

  // Send a physical key up event for the given Key
  bool keyUpKey(Key key) {
    CGKeyCode keyCode = layoutMapper.getKeyCode(key);
    if (debugBackendEnabled()) {
      qDebug() << "[backend::macos] keyUpKey:" << QString::fromStdString(keyToString(key))
               << "keyCode:" << (keyCode == UINT16_MAX ? QStringLiteral("<none>") : QString::number(keyCode));
    }
    if (keyCode == UINT16_MAX) return false;
    return postKeyEvent(keyCode, false, 0);
  }

  // Simple tap: down then up
  bool tapKey(Key key) {
    return keyDownKey(key) && keyUpKey(key);
  }

  // Handle KeyStroke (now only contains Key)
  bool keyDown(const KeyStroke &keystroke) {
    if (debugBackendEnabled()) {
      qDebug() << "[backend::macos] keyDown(keystroke):" << QString::fromStdString(keyToString(keystroke.key));
    }
    return keyDownKey(keystroke.key);
  }

  bool keyUp(const KeyStroke &keystroke) {
    if (debugBackendEnabled()) {
      qDebug() << "[backend::macos] keyUp(keystroke):" << QString::fromStdString(keyToString(keystroke.key));
    }
    return keyUpKey(keystroke.key);
  }

  bool tap(const KeyStroke &keystroke) {
    if (debugBackendEnabled()) {
      qDebug() << "[backend::macos] tap(keystroke):" << QString::fromStdString(keyToString(keystroke.key));
    }
    return tapKey(keystroke.key);
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

bool InputBackend::requestPermissions() {yyyu
  return m_impl && m_impl->requestPermissions();
}

bool InputBackend::keyDown(const KeyStroke &keystroke) {
  return m_impl && m_impl->keyDown(keystroke);
}

bool InputBackend::keyUp(const KeyStroke &keystroke) {
  return m_impl && m_impl->keyUp(keystroke);
}

bool InputBackend::tap(const KeyStroke &keystroke) {
  return m_impl && m_impl->tap(keystroke);
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
