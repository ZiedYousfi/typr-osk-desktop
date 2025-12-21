#ifdef _WIN32

#include "backend.hpp"
#include <Windows.h>
#include <unordered_map>
#include <thread>
#include <chrono>

namespace backend {

namespace {

// Map our Key enum to Windows Virtual Key codes
WORD keyToVK(Key key) {
  static const std::unordered_map<Key, WORD> map = {
    // Letters
    {Key::A, 'A'}, {Key::B, 'B'}, {Key::C, 'C'}, {Key::D, 'D'},
    {Key::E, 'E'}, {Key::F, 'F'}, {Key::G, 'G'}, {Key::H, 'H'},
    {Key::I, 'I'}, {Key::J, 'J'}, {Key::K, 'K'}, {Key::L, 'L'},
    {Key::M, 'M'}, {Key::N, 'N'}, {Key::O, 'O'}, {Key::P, 'P'},
    {Key::Q, 'Q'}, {Key::R, 'R'}, {Key::S, 'S'}, {Key::T, 'T'},
    {Key::U, 'U'}, {Key::V, 'V'}, {Key::W, 'W'}, {Key::X, 'X'},
    {Key::Y, 'Y'}, {Key::Z, 'Z'},
    // Numbers
    {Key::Num0, '0'}, {Key::Num1, '1'}, {Key::Num2, '2'}, {Key::Num3, '3'},
    {Key::Num4, '4'}, {Key::Num5, '5'}, {Key::Num6, '6'}, {Key::Num7, '7'},
    {Key::Num8, '8'}, {Key::Num9, '9'},
    // Function keys
    {Key::F1, VK_F1}, {Key::F2, VK_F2}, {Key::F3, VK_F3}, {Key::F4, VK_F4},
    {Key::F5, VK_F5}, {Key::F6, VK_F6}, {Key::F7, VK_F7}, {Key::F8, VK_F8},
    {Key::F9, VK_F9}, {Key::F10, VK_F10}, {Key::F11, VK_F11}, {Key::F12, VK_F12},
    {Key::F13, VK_F13}, {Key::F14, VK_F14}, {Key::F15, VK_F15}, {Key::F16, VK_F16},
    {Key::F17, VK_F17}, {Key::F18, VK_F18}, {Key::F19, VK_F19}, {Key::F20, VK_F20},
    // Control
    {Key::Enter, VK_RETURN}, {Key::Escape, VK_ESCAPE}, {Key::Backspace, VK_BACK},
    {Key::Tab, VK_TAB}, {Key::Space, VK_SPACE},
    // Navigation
    {Key::Left, VK_LEFT}, {Key::Right, VK_RIGHT}, {Key::Up, VK_UP}, {Key::Down, VK_DOWN},
    {Key::Home, VK_HOME}, {Key::End, VK_END}, {Key::PageUp, VK_PRIOR}, {Key::PageDown, VK_NEXT},
    {Key::Delete, VK_DELETE}, {Key::Insert, VK_INSERT},
    {Key::PrintScreen, VK_SNAPSHOT}, {Key::ScrollLock, VK_SCROLL}, {Key::Pause, VK_PAUSE},
    // Numpad
    {Key::Numpad0, VK_NUMPAD0}, {Key::Numpad1, VK_NUMPAD1}, {Key::Numpad2, VK_NUMPAD2},
    {Key::Numpad3, VK_NUMPAD3}, {Key::Numpad4, VK_NUMPAD4}, {Key::Numpad5, VK_NUMPAD5},
    {Key::Numpad6, VK_NUMPAD6}, {Key::Numpad7, VK_NUMPAD7}, {Key::Numpad8, VK_NUMPAD8},
    {Key::Numpad9, VK_NUMPAD9},
    {Key::NumpadDivide, VK_DIVIDE}, {Key::NumpadMultiply, VK_MULTIPLY},
    {Key::NumpadMinus, VK_SUBTRACT}, {Key::NumpadPlus, VK_ADD},
    {Key::NumpadEnter, VK_RETURN}, {Key::NumpadDecimal, VK_DECIMAL},
    // Modifiers
    {Key::ShiftLeft, VK_LSHIFT}, {Key::ShiftRight, VK_RSHIFT},
    {Key::CtrlLeft, VK_LCONTROL}, {Key::CtrlRight, VK_RCONTROL},
    {Key::AltLeft, VK_LMENU}, {Key::AltRight, VK_RMENU},
    {Key::SuperLeft, VK_LWIN}, {Key::SuperRight, VK_RWIN},
    {Key::CapsLock, VK_CAPITAL}, {Key::NumLock, VK_NUMLOCK},
    // Misc
    {Key::Menu, VK_APPS},
    {Key::Mute, VK_VOLUME_MUTE}, {Key::VolumeDown, VK_VOLUME_DOWN}, {Key::VolumeUp, VK_VOLUME_UP},
    {Key::MediaPlayPause, VK_MEDIA_PLAY_PAUSE}, {Key::MediaStop, VK_MEDIA_STOP},
    {Key::MediaNext, VK_MEDIA_NEXT_TRACK}, {Key::MediaPrevious, VK_MEDIA_PREV_TRACK},
    // Punctuation
    {Key::Grave, VK_OEM_3}, {Key::Minus, VK_OEM_MINUS}, {Key::Equal, VK_OEM_PLUS},
    {Key::LeftBracket, VK_OEM_4}, {Key::RightBracket, VK_OEM_6}, {Key::Backslash, VK_OEM_5},
    {Key::Semicolon, VK_OEM_1}, {Key::Apostrophe, VK_OEM_7},
    {Key::Comma, VK_OEM_COMMA}, {Key::Period, VK_OEM_PERIOD}, {Key::Slash, VK_OEM_2},
  };
  
  auto it = map.find(key);
  return (it != map.end()) ? it->second : 0;
}

// Check if key needs EXTENDEDKEY flag
bool isExtendedKey(WORD vk) {
  switch (vk) {
    case VK_INSERT: case VK_DELETE: case VK_HOME: case VK_END:
    case VK_PRIOR: case VK_NEXT: case VK_LEFT: case VK_RIGHT:
    case VK_UP: case VK_DOWN: case VK_SNAPSHOT: case VK_DIVIDE:
    case VK_NUMLOCK: case VK_RCONTROL: case VK_RMENU:
    case VK_LWIN: case VK_RWIN: case VK_APPS:
      return true;
    default:
      return false;
  }
}

} // namespace

struct InputBackend::Impl {
  Modifier currentMods{Modifier::None};
  uint32_t keyDelayUs{1000}; // 1ms default

  bool sendKey(Key key, bool down) {
    WORD vk = keyToVK(key);
    if (vk == 0) return false;

    INPUT input{};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    input.ki.wScan = MapVirtualKeyW(vk, MAPVK_VK_TO_VSC);
    input.ki.dwFlags = KEYEVENTF_SCANCODE;
    
    if (isExtendedKey(vk)) {
      input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    }
    if (!down) {
      input.ki.dwFlags |= KEYEVENTF_KEYUP;
    }

    return SendInput(1, &input, sizeof(INPUT)) > 0;
  }

  bool typeUnicode(const std::u32string& text) {
    if (text.empty()) return true;

    std::vector<INPUT> inputs;
    inputs.reserve(text.size() * 4); // Worst case: surrogate pairs + up/down

    for (char32_t cp : text) {
      // Convert to UTF-16
      wchar_t utf16[2];
      int len = 0;
      
      if (cp <= 0xFFFF) {
        utf16[0] = static_cast<wchar_t>(cp);
        len = 1;
      } else if (cp <= 0x10FFFF) {
        cp -= 0x10000;
        utf16[0] = static_cast<wchar_t>(0xD800 | (cp >> 10));
        utf16[1] = static_cast<wchar_t>(0xDC00 | (cp & 0x3FF));
        len = 2;
      } else {
        continue; // Invalid codepoint
      }

      for (int i = 0; i < len; ++i) {
        INPUT down{}, up{};
        down.type = INPUT_KEYBOARD;
        down.ki.wScan = utf16[i];
        down.ki.dwFlags = KEYEVENTF_UNICODE;
        
        up = down;
        up.ki.dwFlags |= KEYEVENTF_KEYUP;
        
        inputs.push_back(down);
        inputs.push_back(up);
      }
    }

    return SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT)) > 0;
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

BackendType InputBackend::type() const { return BackendType::Windows; }

Capabilities InputBackend::capabilities() const {
  return {
    .canInjectKeys = true,
    .canInjectText = true,
    .canSimulateHID = true,  // SendInput with scancodes is HID-level
    .supportsKeyRepeat = true,
    .needsAccessibilityPerm = false,
    .needsInputMonitoringPerm = false,
    .needsUinputAccess = false,
  };
}

bool InputBackend::isReady() const { return true; }
bool InputBackend::requestPermissions() { return true; }

bool InputBackend::keyDown(Key key) {
  return m_impl->sendKey(key, true);
}

bool InputBackend::keyUp(Key key) {
  return m_impl->sendKey(key, false);
}

bool InputBackend::tap(Key key) {
  if (!keyDown(key)) return false;
  m_impl->delay();
  return keyUp(key);
}

Modifier InputBackend::activeModifiers() const {
  return m_impl->currentMods;
}

bool InputBackend::holdModifier(Modifier mod) {
  bool ok = true;
  if (hasModifier(mod, Modifier::Shift) && !hasModifier(m_impl->currentMods, Modifier::Shift)) {
    ok &= keyDown(Key::ShiftLeft);
  }
  if (hasModifier(mod, Modifier::Ctrl) && !hasModifier(m_impl->currentMods, Modifier::Ctrl)) {
    ok &= keyDown(Key::CtrlLeft);
  }
  if (hasModifier(mod, Modifier::Alt) && !hasModifier(m_impl->currentMods, Modifier::Alt)) {
    ok &= keyDown(Key::AltLeft);
  }
  if (hasModifier(mod, Modifier::Super) && !hasModifier(m_impl->currentMods, Modifier::Super)) {
    ok &= keyDown(Key::SuperLeft);
  }
  m_impl->currentMods = m_impl->currentMods | mod;
  return ok;
}

bool InputBackend::releaseModifier(Modifier mod) {
  bool ok = true;
  if (hasModifier(mod, Modifier::Shift) && hasModifier(m_impl->currentMods, Modifier::Shift)) {
    ok &= keyUp(Key::ShiftLeft);
  }
  if (hasModifier(mod, Modifier::Ctrl) && hasModifier(m_impl->currentMods, Modifier::Ctrl)) {
    ok &= keyUp(Key::CtrlLeft);
  }
  if (hasModifier(mod, Modifier::Alt) && hasModifier(m_impl->currentMods, Modifier::Alt)) {
    ok &= keyUp(Key::AltLeft);
  }
  if (hasModifier(mod, Modifier::Super) && hasModifier(m_impl->currentMods, Modifier::Super)) {
    ok &= keyUp(Key::SuperLeft);
  }
  m_impl->currentMods = static_cast<Modifier>(
    static_cast<uint8_t>(m_impl->currentMods) & ~static_cast<uint8_t>(mod)
  );
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
  // Convert UTF-8 to UTF-32
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
  // Windows SendInput is synchronous, nothing to flush
}

void InputBackend::setKeyDelay(uint32_t delayUs) {
  m_impl->keyDelayUs = delayUs;
}

} // namespace backend

#endif // _WIN32