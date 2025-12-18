// input_windows.cpp
// Windows backend for keyboard input injection using SendInput API

#include "input.hpp"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <string>
#include <vector>
#include <windows.h>

namespace input {

namespace {

// Convert input::Key to Windows Virtual-Key code
WORD keyToWinVK(Key key) {
  switch (key) {
  // Letters
  case Key::A:
    return 'A';
  case Key::B:
    return 'B';
  case Key::C:
    return 'C';
  case Key::D:
    return 'D';
  case Key::E:
    return 'E';
  case Key::F:
    return 'F';
  case Key::G:
    return 'G';
  case Key::H:
    return 'H';
  case Key::I:
    return 'I';
  case Key::J:
    return 'J';
  case Key::K:
    return 'K';
  case Key::L:
    return 'L';
  case Key::M:
    return 'M';
  case Key::N:
    return 'N';
  case Key::O:
    return 'O';
  case Key::P:
    return 'P';
  case Key::Q:
    return 'Q';
  case Key::R:
    return 'R';
  case Key::S:
    return 'S';
  case Key::T:
    return 'T';
  case Key::U:
    return 'U';
  case Key::V:
    return 'V';
  case Key::W:
    return 'W';
  case Key::X:
    return 'X';
  case Key::Y:
    return 'Y';
  case Key::Z:
    return 'Z';

  // Numbers
  case Key::Num0:
    return '0';
  case Key::Num1:
    return '1';
  case Key::Num2:
    return '2';
  case Key::Num3:
    return '3';
  case Key::Num4:
    return '4';
  case Key::Num5:
    return '5';
  case Key::Num6:
    return '6';
  case Key::Num7:
    return '7';
  case Key::Num8:
    return '8';
  case Key::Num9:
    return '9';

  // Function keys
  case Key::F1:
    return VK_F1;
  case Key::F2:
    return VK_F2;
  case Key::F3:
    return VK_F3;
  case Key::F4:
    return VK_F4;
  case Key::F5:
    return VK_F5;
  case Key::F6:
    return VK_F6;
  case Key::F7:
    return VK_F7;
  case Key::F8:
    return VK_F8;
  case Key::F9:
    return VK_F9;
  case Key::F10:
    return VK_F10;
  case Key::F11:
    return VK_F11;
  case Key::F12:
    return VK_F12;
  case Key::F13:
    return VK_F13;
  case Key::F14:
    return VK_F14;
  case Key::F15:
    return VK_F15;
  case Key::F16:
    return VK_F16;
  case Key::F17:
    return VK_F17;
  case Key::F18:
    return VK_F18;
  case Key::F19:
    return VK_F19;
  case Key::F20:
    return VK_F20;

  // Control keys
  case Key::Enter:
    return VK_RETURN;
  case Key::Escape:
    return VK_ESCAPE;
  case Key::Backspace:
    return VK_BACK;
  case Key::Tab:
    return VK_TAB;
  case Key::Space:
    return VK_SPACE;
  case Key::Delete:
    return VK_DELETE;
  case Key::Insert:
    return VK_INSERT;
  case Key::Pause:
    return VK_PAUSE;

  // Navigation
  case Key::Left:
    return VK_LEFT;
  case Key::Right:
    return VK_RIGHT;
  case Key::Up:
    return VK_UP;
  case Key::Down:
    return VK_DOWN;
  case Key::Home:
    return VK_HOME;
  case Key::End:
    return VK_END;
  case Key::PageUp:
    return VK_PRIOR;
  case Key::PageDown:
    return VK_NEXT;

  // Punctuation (Layout dependent, but SendInput usually works with VKs)
  case Key::Grave:
    return VK_OEM_3;
  case Key::Minus:
    return VK_OEM_MINUS;
  case Key::Equal:
    return VK_OEM_PLUS;
  case Key::LeftBracket:
    return VK_OEM_4;
  case Key::RightBracket:
    return VK_OEM_6;
  case Key::Backslash:
    return VK_OEM_5;
  case Key::Semicolon:
    return VK_OEM_1;
  case Key::Apostrophe:
    return VK_OEM_7;
  case Key::Comma:
    return VK_OEM_COMMA;
  case Key::Period:
    return VK_OEM_PERIOD;
  case Key::Slash:
    return VK_OEM_2;

  // Numpad
  case Key::Numpad0:
    return VK_NUMPAD0;
  case Key::Numpad1:
    return VK_NUMPAD1;
  case Key::Numpad2:
    return VK_NUMPAD2;
  case Key::Numpad3:
    return VK_NUMPAD3;
  case Key::Numpad4:
    return VK_NUMPAD4;
  case Key::Numpad5:
    return VK_NUMPAD5;
  case Key::Numpad6:
    return VK_NUMPAD6;
  case Key::Numpad7:
    return VK_NUMPAD7;
  case Key::Numpad8:
    return VK_NUMPAD8;
  case Key::Numpad9:
    return VK_NUMPAD9;
  case Key::NumpadDivide:
    return VK_DIVIDE;
  case Key::NumpadMultiply:
    return VK_MULTIPLY;
  case Key::NumpadMinus:
    return VK_SUBTRACT;
  case Key::NumpadPlus:
    return VK_ADD;
  case Key::NumpadDecimal:
    return VK_DECIMAL;
  case Key::NumpadEnter:
    return VK_RETURN; // Note: No specific VK for Numpad Enter

  // Modifiers
  case Key::ShiftLeft:
    return VK_LSHIFT;
  case Key::ShiftRight:
    return VK_RSHIFT;
  case Key::CtrlLeft:
    return VK_LCONTROL;
  case Key::CtrlRight:
    return VK_RCONTROL;
  case Key::AltLeft:
    return VK_LMENU;
  case Key::AltRight:
    return VK_RMENU;
  case Key::SuperLeft:
    return VK_LWIN;
  case Key::SuperRight:
    return VK_RWIN;

  // Locks
  case Key::CapsLock:
    return VK_CAPITAL;
  case Key::NumLock:
    return VK_NUMLOCK;
  case Key::ScrollLock:
    return VK_SCROLL;

  // Media/Special
  case Key::PrintScreen:
    return VK_SNAPSHOT;
  case Key::Menu:
    return VK_APPS;
  case Key::Mute:
    return VK_VOLUME_MUTE;
  case Key::VolumeDown:
    return VK_VOLUME_DOWN;
  case Key::VolumeUp:
    return VK_VOLUME_UP;
  case Key::MediaPlayPause:
    return VK_MEDIA_PLAY_PAUSE;
  case Key::MediaStop:
    return VK_MEDIA_STOP;
  case Key::MediaNext:
    return VK_MEDIA_NEXT_TRACK;
  case Key::MediaPrevious:
    return VK_MEDIA_PREV_TRACK;

  default:
    return 0;
  }
}

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
  Impl() {}

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

  bool keyDown(Key key, Mod mods) {
    std::vector<INPUT> inputs;
    addModifierInputs(inputs, mods, true);

    WORD vk = keyToWinVK(key);
    if (vk != 0) {
      INPUT input = {0};
      input.type = INPUT_KEYBOARD;
      input.ki.wVk = vk;
      input.ki.dwFlags = isExtendedKey(vk) ? KEYEVENTF_EXTENDEDKEY : 0;
      inputs.push_back(input);
    }

    return sendInputs(inputs);
  }

  bool keyUp(Key key, Mod mods) {
    std::vector<INPUT> inputs;

    WORD vk = keyToWinVK(key);
    if (vk != 0) {
      INPUT input = {0};
      input.type = INPUT_KEYBOARD;
      input.ki.wVk = vk;
      input.ki.dwFlags =
          KEYEVENTF_KEYUP | (isExtendedKey(vk) ? KEYEVENTF_EXTENDEDKEY : 0);
      inputs.push_back(input);
    }

    addModifierInputs(inputs, mods, false);
    return sendInputs(inputs);
  }

  bool tap(Key key, Mod mods) {
    std::vector<INPUT> inputs;

    // Press modifiers
    addModifierInputs(inputs, mods, true);

    WORD vk = keyToWinVK(key);
    if (vk != 0) {
      // Key down
      INPUT down = {0};
      down.type = INPUT_KEYBOARD;
      down.ki.wVk = vk;
      down.ki.dwFlags = isExtendedKey(vk) ? KEYEVENTF_EXTENDEDKEY : 0;
      inputs.push_back(down);

      // Key up
      INPUT up = down;
      up.ki.dwFlags |= KEYEVENTF_KEYUP;
      inputs.push_back(up);
    }

    // Release modifiers
    addModifierInputs(inputs, mods, false);

    return sendInputs(inputs);
  }

  bool keyDown(const KeyStroke &keystroke) {
    if (keystroke.character && !keystroke.character->empty()) {
      return typeText(*keystroke.character);
    }
    return keyDown(keystroke.key, keystroke.mods);
  }

  bool keyUp(const KeyStroke &keystroke) {
    if (keystroke.character && !keystroke.character->empty()) {
      return true;
    }
    return keyUp(keystroke.key, keystroke.mods);
  }

  bool tap(const KeyStroke &keystroke) {
    if (keystroke.character && !keystroke.character->empty()) {
      return typeText(*keystroke.character);
    }
    return tap(keystroke.key, keystroke.mods);
  }

  bool typeText(const std::u32string &text) {
    std::wstring wstr = utf32ToWString(text);
    if (wstr.empty())
      return true;

    std::vector<INPUT> inputs;
    inputs.reserve(wstr.size() * 2);

    for (wchar_t ch : wstr) {
      INPUT down = {0};
      down.type = INPUT_KEYBOARD;
      down.ki.wScan = ch;
      down.ki.dwFlags = KEYEVENTF_UNICODE;
      inputs.push_back(down);

      INPUT up = down;
      up.ki.dwFlags |= KEYEVENTF_KEYUP;
      inputs.push_back(up);
    }

    return sendInputs(inputs);
  }

  bool typeText(const std::string &utf8Text) {
    std::wstring wstr = utf8ToWString(utf8Text);
    if (wstr.empty())
      return true;

    std::vector<INPUT> inputs;
    inputs.reserve(wstr.size() * 2);

    for (wchar_t ch : wstr) {
      INPUT down = {0};
      down.type = INPUT_KEYBOARD;
      down.ki.wScan = ch;
      down.ki.dwFlags = KEYEVENTF_UNICODE;
      inputs.push_back(down);

      INPUT up = down;
      up.ki.dwFlags |= KEYEVENTF_KEYUP;
      inputs.push_back(up);
    }

    return sendInputs(inputs);
  }

  bool typeCharacter(char32_t codepoint) {
    return typeText(std::u32string(1, codepoint));
  }

private:
  void addModifierInputs(std::vector<INPUT> &inputs, Mod mods, bool down) {
    DWORD flags = down ? 0 : KEYEVENTF_KEYUP;

    if (mods & Mod_Shift) {
      INPUT input = {0};
      input.type = INPUT_KEYBOARD;
      input.ki.wVk = VK_SHIFT;
      input.ki.dwFlags = flags;
      inputs.push_back(input);
    }
    if (mods & Mod_Ctrl) {
      INPUT input = {0};
      input.type = INPUT_KEYBOARD;
      input.ki.wVk = VK_CONTROL;
      input.ki.dwFlags = flags;
      inputs.push_back(input);
    }
    if (mods & Mod_Alt) {
      INPUT input = {0};
      input.type = INPUT_KEYBOARD;
      input.ki.wVk = VK_MENU;
      input.ki.dwFlags = flags;
      inputs.push_back(input);
    }
    if (mods & Mod_Super) {
      INPUT input = {0};
      input.type = INPUT_KEYBOARD;
      input.ki.wVk = VK_LWIN;
      input.ki.dwFlags = flags;
      inputs.push_back(input);
    }
  }
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
bool InputBackend::keyDown(Key k, Mod m) {
  return m_impl && m_impl->keyDown(k, m);
}
bool InputBackend::keyUp(Key k, Mod m) { return m_impl && m_impl->keyUp(k, m); }
bool InputBackend::tap(const KeyStroke &ks) {
  return m_impl && m_impl->tap(ks);
}
bool InputBackend::tap(Key k, Mod m) { return m_impl && m_impl->tap(k, m); }
bool InputBackend::typeText(const std::u32string &t) {
  return m_impl && m_impl->typeText(t);
}
bool InputBackend::typeText(const std::string &t) {
  return m_impl && m_impl->typeText(t);
}
bool InputBackend::typeCharacter(char32_t c) {
  return m_impl && m_impl->typeCharacter(c);
}

} // namespace input

#endif // _WIN32
