// Windows backend for keyboard input injection using SendInput API

#ifdef _WIN32
#include "backend.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <QDebug>
#include <QString>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <windows.h>

namespace backend {

namespace {

// Convert UTF-32 string to UTF-16 (Windows native)
std::wstring utf32ToWString(const std::u32string &text) {
  std::wstring result;
  for (char32_t cp : text) {
    if (cp <= 0xFFFF) {
      result += static_cast<wchar_t>(cp);
    } else {
      cp -= 0x10000;
      result += static_cast<wchar_t>(0xD800 | (cp >> 10));
      result += static_cast<wchar_t>(0xDC00 | (cp & 0x3FF));
    }
  }
  return result;
}

// Convert UTF-8 to UTF-16
std::wstring utf8ToWString(const std::string &utf8) {
  if (utf8.empty())
    return L"";
  int size_needed =
      MultiByteToWideChar(CP_UTF8, 0, &utf8[0], (int)utf8.size(), NULL, 0);
  std::wstring wstrTo(size_needed, 0);
  MultiByteToWideChar(CP_UTF8, 0, &utf8[0], (int)utf8.size(), &wstrTo[0],
                      size_needed);
  return wstrTo;
}

// Debug helper utilities
// Enable backend debug logs with environment var TYPR_OSK_DEBUG_BACKEND
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
    } else if (cp == '\\n') {
      out += "\\n";
    } else {
      char buf[16];
      snprintf(buf, sizeof(buf), "\\u%04x", (unsigned)cp);
      out += buf;
    }
    if (out.size() > 64) {
      out += "...";
      break;
    }
  }
  if (out.empty())
    out = "";
  return out;
}

static void logTypeText(const std::u32string &text) {
  if (!debugBackendEnabled())
    return;
  std::string preview = debugPreviewFromUtf32(text);
  qDebug() << "[backend::win] typeText:"
           << "length=" << (int)text.size()
           << "preview=" << QString::fromStdString(preview);
}

// Helper to normalize characters for comparison (lowercase)
wchar_t normalizeChar(wchar_t c) {
  if (c >= L'A' && c <= L'Z') {
    return c - L'A' + L'a';
  }
  return c;
}

// Query the system to find what character a virtual key produces
wchar_t getCharacterForVirtualKey(WORD vk, HKL layout) {
  // Get the scan code for this virtual key
  UINT scanCode = MapVirtualKeyExW(vk, MAPVK_VK_TO_VSC, layout);
  if (scanCode == 0) {
    return 0;
  }

  // Use ToUnicodeEx to get the character
  BYTE keyboardState[256] = {0};
  wchar_t buffer[4] = {0};

  int result = ToUnicodeEx(vk, scanCode, keyboardState, buffer, 4, 0, layout);

  if (result == 1) {
    return buffer[0];
  }

  return 0;
}

// Build a mapping from our Key enum to Windows virtual key codes by scanning
// the keyboard layout
class KeyboardLayoutMapper {
public:
  KeyboardLayoutMapper() { buildMappings(); }

  WORD getVirtualKey(Key key) const {
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

    return 0;
  }

private:
  void buildMappings() {
    // First, set up fixed mappings for non-character keys (these don't change
    // with layout)
    m_fixedMappings[Key::Enter] = VK_RETURN;
    m_fixedMappings[Key::Escape] = VK_ESCAPE;
    m_fixedMappings[Key::Backspace] = VK_BACK;
    m_fixedMappings[Key::Tab] = VK_TAB;
    m_fixedMappings[Key::Space] = VK_SPACE;
    m_fixedMappings[Key::Delete] = VK_DELETE;
    m_fixedMappings[Key::Insert] = VK_INSERT;
    m_fixedMappings[Key::Pause] = VK_PAUSE;
    m_fixedMappings[Key::Left] = VK_LEFT;
    m_fixedMappings[Key::Right] = VK_RIGHT;
    m_fixedMappings[Key::Up] = VK_UP;
    m_fixedMappings[Key::Down] = VK_DOWN;
    m_fixedMappings[Key::Home] = VK_HOME;
    m_fixedMappings[Key::End] = VK_END;
    m_fixedMappings[Key::PageUp] = VK_PRIOR;
    m_fixedMappings[Key::PageDown] = VK_NEXT;
    m_fixedMappings[Key::F1] = VK_F1;
    m_fixedMappings[Key::F2] = VK_F2;
    m_fixedMappings[Key::F3] = VK_F3;
    m_fixedMappings[Key::F4] = VK_F4;
    m_fixedMappings[Key::F5] = VK_F5;
    m_fixedMappings[Key::F6] = VK_F6;
    m_fixedMappings[Key::F7] = VK_F7;
    m_fixedMappings[Key::F8] = VK_F8;
    m_fixedMappings[Key::F9] = VK_F9;
    m_fixedMappings[Key::F10] = VK_F10;
    m_fixedMappings[Key::F11] = VK_F11;
    m_fixedMappings[Key::F12] = VK_F12;
    m_fixedMappings[Key::F13] = VK_F13;
    m_fixedMappings[Key::F14] = VK_F14;
    m_fixedMappings[Key::F15] = VK_F15;
    m_fixedMappings[Key::F16] = VK_F16;
    m_fixedMappings[Key::F17] = VK_F17;
    m_fixedMappings[Key::F18] = VK_F18;
    m_fixedMappings[Key::F19] = VK_F19;
    m_fixedMappings[Key::F20] = VK_F20;
    m_fixedMappings[Key::ShiftLeft] = VK_LSHIFT;
    m_fixedMappings[Key::ShiftRight] = VK_RSHIFT;
    m_fixedMappings[Key::CtrlLeft] = VK_LCONTROL;
    m_fixedMappings[Key::CtrlRight] = VK_RCONTROL;
    m_fixedMappings[Key::AltLeft] = VK_LMENU;
    m_fixedMappings[Key::AltRight] = VK_RMENU;
    m_fixedMappings[Key::SuperLeft] = VK_LWIN;
    m_fixedMappings[Key::SuperRight] = VK_RWIN;
    m_fixedMappings[Key::CapsLock] = VK_CAPITAL;
    m_fixedMappings[Key::NumLock] = VK_NUMLOCK;
    m_fixedMappings[Key::ScrollLock] = VK_SCROLL;
    m_fixedMappings[Key::PrintScreen] = VK_SNAPSHOT;
    m_fixedMappings[Key::Menu] = VK_APPS;
    m_fixedMappings[Key::Mute] = VK_VOLUME_MUTE;
    m_fixedMappings[Key::VolumeDown] = VK_VOLUME_DOWN;
    m_fixedMappings[Key::VolumeUp] = VK_VOLUME_UP;
    m_fixedMappings[Key::MediaPlayPause] = VK_MEDIA_PLAY_PAUSE;
    m_fixedMappings[Key::MediaStop] = VK_MEDIA_STOP;
    m_fixedMappings[Key::MediaNext] = VK_MEDIA_NEXT_TRACK;
    m_fixedMappings[Key::MediaPrevious] = VK_MEDIA_PREV_TRACK;
    m_fixedMappings[Key::Numpad0] = VK_NUMPAD0;
    m_fixedMappings[Key::Numpad1] = VK_NUMPAD1;
    m_fixedMappings[Key::Numpad2] = VK_NUMPAD2;
    m_fixedMappings[Key::Numpad3] = VK_NUMPAD3;
    m_fixedMappings[Key::Numpad4] = VK_NUMPAD4;
    m_fixedMappings[Key::Numpad5] = VK_NUMPAD5;
    m_fixedMappings[Key::Numpad6] = VK_NUMPAD6;
    m_fixedMappings[Key::Numpad7] = VK_NUMPAD7;
    m_fixedMappings[Key::Numpad8] = VK_NUMPAD8;
    m_fixedMappings[Key::Numpad9] = VK_NUMPAD9;
    m_fixedMappings[Key::NumpadDivide] = VK_DIVIDE;
    m_fixedMappings[Key::NumpadMultiply] = VK_MULTIPLY;
    m_fixedMappings[Key::NumpadMinus] = VK_SUBTRACT;
    m_fixedMappings[Key::NumpadPlus] = VK_ADD;
    m_fixedMappings[Key::NumpadDecimal] = VK_DECIMAL;
    m_fixedMappings[Key::NumpadEnter] = VK_RETURN;

    // Get current keyboard layout
    HKL layout = GetKeyboardLayout(0);

    // Now scan the keyboard layout to find which physical keys produce which
    // characters We'll scan common virtual key codes
    for (WORD vk = 0; vk < 256; ++vk) {
      wchar_t c = getCharacterForVirtualKey(vk, layout);
      if (c == 0)
        continue;

      c = normalizeChar(c);

      // Map letters A-Z
      if (c >= L'a' && c <= L'z') {
        Key key = static_cast<Key>(static_cast<uint16_t>(Key::A) + (c - L'a'));
        if (m_keyToVirtualKey.find(key) == m_keyToVirtualKey.end()) {
          m_keyToVirtualKey[key] = vk;
        }
      }
      // Map numbers 0-9
      else if (c >= L'0' && c <= L'9') {
        Key key =
            static_cast<Key>(static_cast<uint16_t>(Key::Num0) + (c - L'0'));
        if (m_keyToVirtualKey.find(key) == m_keyToVirtualKey.end()) {
          m_keyToVirtualKey[key] = vk;
        }
      }
      // Map punctuation and special characters
      else {
        Key key = Key::Unknown;
        switch (c) {
        case L'`':
          key = Key::Grave;
          break;
        case L'-':
          key = Key::Minus;
          break;
        case L'=':
          key = Key::Equal;
          break;
        case L'[':
          key = Key::LeftBracket;
          break;
        case L']':
          key = Key::RightBracket;
          break;
        case L'\\':
          key = Key::Backslash;
          break;
        case L';':
          key = Key::Semicolon;
          break;
        case L'\'':
          key = Key::Apostrophe;
          break;
        case L',':
          key = Key::Comma;
          break;
        case L'.':
          key = Key::Period;
          break;
        case L'/':
          key = Key::Slash;
          break;
        default:
          break;
        }
        if (key != Key::Unknown &&
            m_keyToVirtualKey.find(key) == m_keyToVirtualKey.end()) {
          m_keyToVirtualKey[key] = vk;
        }
      }
    }
  }

  std::unordered_map<Key, WORD> m_fixedMappings;
  std::unordered_map<Key, WORD> m_keyToVirtualKey;
};

// Check if a virtual key code requires the Extended Key flag
bool isExtendedKey(WORD vk) {
  switch (vk) {
  case VK_INSERT:
  case VK_DELETE:
  case VK_HOME:
  case VK_END:
  case VK_PRIOR:
  case VK_NEXT:
  case VK_LEFT:
  case VK_RIGHT:
  case VK_UP:
  case VK_DOWN:
  case VK_DIVIDE:
  case VK_RMENU:
  case VK_RCONTROL:
  case VK_LWIN:
  case VK_RWIN:
  case VK_APPS:
    return true;
  default:
    return false;
  }
}

bool sendInputs(const std::vector<INPUT> &inputs) {
  if (inputs.empty())
    return true;
  UINT sent = SendInput((UINT)inputs.size(), const_cast<LPINPUT>(inputs.data()),
                        sizeof(INPUT));
  return sent == inputs.size();
}

} // namespace

struct InputBackend::Impl {
  KeyboardLayoutMapper layoutMapper;

  Impl() {
    if (debugBackendEnabled()) {
      qDebug() << "[backend::win] InputBackend::Impl initialized";
    }
  }

  Capabilities capabilities() const {
    Capabilities caps;
    caps.canInjectEvents = true;
    caps.canActAsHID = false;
    caps.needsAccessibilityPerm = false;
    caps.needsInputMonitoringPerm = false;
    return caps;
  }

  bool isReady() const { return true; }

  bool requestPermissions() { return true; }

  bool keyDown(const KeyStroke &keystroke) {
    // Simple physical key down using mapping (modifiers are represented as keys
    // themselves)
    qDebug() << "[backend::win] keyDown(keystroke) entry:"
             << QString::fromStdString(keyToString(keystroke.key));
    bool ok = keyDownKey(keystroke.key);
    qDebug() << "[backend::win] keyDown: result:" << ok;
    if (!ok)
      qWarning() << "[backend::win] keyDown: physical keyDown failed for"
                 << QString::fromStdString(keyToString(keystroke.key));
    return ok;
  }

  bool keyUp(const KeyStroke &keystroke) {
    // Simple physical key up (modifiers are sent as independent keys)
    if (debugBackendEnabled()) {
      qDebug() << "[backend::win] keyUp(keystroke): physical key"
               << QString::fromStdString(keyToString(keystroke.key));
    }
    return keyUpKey(keystroke.key);
  }

  bool tap(const KeyStroke &keystroke) {
    if (debugBackendEnabled()) {
      qDebug() << "[backend::win] tap(keystroke): physical tap for"
               << QString::fromStdString(keyToString(keystroke.key));
    }
    return tapKey(keystroke.key);
  }

  bool typeCharacter(char32_t codepoint) {
    return typeText(std::u32string(1, codepoint));
  }

private:
  // Send a physical key down (using virtual-key mapping)
  bool keyDownKey(Key key) {
    WORD vk = layoutMapper.getVirtualKey(key);
    if (vk == 0) {
      if (debugBackendEnabled()) {
        qDebug() << "[backend::win] keyDownKey: no mapping for"
                 << QString::fromStdString(keyToString(key));
      }
      return false;
    }
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    if (isExtendedKey(vk)) {
      input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    }
    std::vector<INPUT> inputs;
    inputs.push_back(input);
    bool ok = sendInputs(inputs);
    if (debugBackendEnabled()) {
      qDebug() << "[backend::win] keyDownKey: vk:" << (int)vk << "ok:" << ok;
    }
    return ok;
  }

  // Send a physical key up (using virtual-key mapping)
  bool keyUpKey(Key key) {
    WORD vk = layoutMapper.getVirtualKey(key);
    if (vk == 0) {
      if (debugBackendEnabled()) {
        qDebug() << "[backend::win] keyUpKey: no mapping for"
                 << QString::fromStdString(keyToString(key));
      }
      return false;
    }
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    input.ki.dwFlags =
        KEYEVENTF_KEYUP | (isExtendedKey(vk) ? KEYEVENTF_EXTENDEDKEY : 0);
    std::vector<INPUT> inputs;
    inputs.push_back(input);
    bool ok = sendInputs(inputs);
    if (debugBackendEnabled()) {
      qDebug() << "[backend::win] keyUpKey: vk:" << (int)vk << "ok:" << ok;
    }
    return ok;
  }

  // Simple tap: down then up
  bool tapKey(Key key) { return keyDownKey(key) && keyUpKey(key); }
};

// InputBackend public interface implementation (Standard Pimpl delegation)

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
bool InputBackend::keyDown(const KeyStroke &ks) {
  return m_impl && m_impl->keyDown(ks);
}
bool InputBackend::keyUp(const KeyStroke &ks) {
  return m_impl && m_impl->keyUp(ks);
}
bool InputBackend::tap(const KeyStroke &ks) {
  return m_impl && m_impl->tap(ks);
}
bool InputBackend::typeText(const std::u32string &t) {
  return m_impl && m_impl->typeText(t);
}
bool InputBackend::typeText(const std::string &t) {
  return m_impl && m_impl->typeText(t);
}
bool InputBackend::typeCharacter(char32_t c) {
  return m_impl && m_impl->typeCharacter(c);
}

} // namespace backend

#endif // _WIN32
