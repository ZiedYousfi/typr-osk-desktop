# Input Subsystem

This directory contains the cross-platform input injection backend for Typr OSK. It provides a unified interface to simulate keyboard events across different operating systems.

## Key Handling Philosophy

The "right way" to implement a virtual keyboard is to send low-level hardware scan codes or virtual key codes. This allows the Operating System to handle the translation from a physical key press to a specific character based on the user's active keyboard layout (e.g., QWERTY, AZERTY, Dvorak). This ensures that the virtual keyboard behaves exactly like a physical one.

## Dynamic Layout Mapping (and platform differences)

We prefer to send physical key events (scan codes or virtual key codes) because that preserves native shortcuts, modifier behaviour, and the Operating System's own interpretation of keys. Where possible, backends dynamically scan the system keyboard layout to map our logical `Key` enum to the appropriate physical key code for the user's active layout. In some environments (or when the backend is implemented differently) we fall back to a direct/native `Key`→platform-code mapping instead.

### How dynamic scanning works (when available):

1. **Initialization**: When the `InputBackend` is created, it queries the current system keyboard layout.
2. **Scanning**: The backend enumerates platform key codes and asks the OS: _"If this physical key were pressed, what character would it produce?"_
3. **Mapping**: We build a reverse lookup table mapping logical keys (e.g., `Key::A`) to the platform key code that produces that character in the user's layout.

### Platform differences

- macOS: the backend uses macOS keyboard layout APIs to translate characters to `CGKeyCode` and prefers sending mapped physical key events. macOS also supports direct Unicode injection (`CGEventKeyboardSetUnicodeString`) when needed, but most shortcut semantics are preserved by sending physical keys. Accessibility permissions are required for these capabilities.

- Windows: the backend scans VK codes using `GetKeyboardLayout`, `MapVirtualKeyExW`, and `ToUnicodeEx` to build a reliable mapping. Windows supports both scancode-based (HID-like) injection and direct Unicode insertion (`KEYEVENTF_UNICODE`) for text.

- Linux (uinput): the uinput backend maps our `Key` enum to Linux `KEY_*` codes and creates a virtual device via `/dev/uinput` to emit `EV_KEY` events. This implementation does not perform a per-layout scan; it relies on emitting kernel key codes. Because of that, `typeText` (direct Unicode injection) is not provided by the uinput backend here and `/dev/uinput` access must be granted via udev rules or root privileges.

The overarching goal is consistent across backends: produce events that behave like physical key presses so native shortcuts, OS handling, and modifier semantics remain correct. When emitting physical key events is not possible or not ideal, backends may use direct Unicode/text injection as an alternative.

## Platform Implementation Details

### OutputListener (global keyboard monitoring)

An `OutputListener` has been added to the backend to provide a cross-platform, best-effort way to monitor global keyboard output (what the user types). The listener invokes a callback with four parameters: the produced Unicode codepoint (0 if none), a `Key` enum mapping for the physical key (or `Key::Unknown`), the active `Modifier` bitmask, and a boolean indicating whether the event was a key press (true) or release (false).

Platform support and notes:

- Windows: implemented using a low-level keyboard hook (`WH_KEYBOARD_LL`) and `ToUnicodeEx` for Unicode extraction.
- macOS: implemented with a CGEvent tap (`CGEventTapCreate`) and `CGEventKeyboardGetUnicodeString`. Input Monitoring permission may be required; the listener will fail to start if the system denies it.
- Linux (X11): implemented using XInput2 raw events (XI_RawKeyPress / XI_RawKeyRelease) and XKB lookups for key-to-keysym/character mapping. This requires XInput2; if XInput2 is not available the listener will not start. Wayland is not supported by the current implementation.
- Behaviour: the implementation is intentionally lightweight (complex IME/dead-key handling is not attempted). It focuses on ASCII/BMP printable characters and provides a physical key mapping consistent with the InputBackend's layout-aware mapping.

Usage: construct an `OutputListener` and call `startListening(callback)` to begin receiving events and `stopListening()` to stop. The callback signature is:

`std::function<void(char32_t codepoint, backend::Key key, backend::Modifier mods, bool pressed)>`.

Note: the listener is designed to be low-noise (noisy logging is off by default).

### macOS (`input_macos.mm`)

We use `TISCopyCurrentKeyboardLayoutInputSource` and `UCKeyTranslate` to perform the layout scanning. This allows us to discover the `CGKeyCode` for every character. For complex inputs or cases where translation is preferred, we still support direct Unicode injection via `CGEventKeyboardSetUnicodeString`, but we prioritize physical key events to preserve native OS behavior (like keyboard shortcuts).

Additionally, the macOS backend now physically presses and releases modifier keys (Shift/Ctrl/Alt/Cmd) when sending `keyDown`/`keyUp` for mapped keys. Concretely, when a keystroke includes modifiers and a mapped virtual key is available, the backend will press the appropriate modifier key(s) down before the main key and release them after the main key is released. The implementation prefers left-side modifiers (e.g., `ShiftLeft`, `CtrlLeft`) when available and falls back to right-side variants if necessary.

This change brings macOS behavior into parity with the Windows backend (which already sends explicit modifier key down/up events) and improves compatibility with applications that expect explicit modifier press/release events (rather than relying only on modifier flags in a single key event). Note that direct Unicode injection via `CGEventKeyboardSetUnicodeString` (used for arbitrary characters that don't map to a physical key) still injects characters directly and does not physically press modifier keys.

### Windows (`input_windows.cpp`)

We use `GetKeyboardLayout`, `MapVirtualKeyExW`, and `ToUnicodeEx` to scan the virtual key space (VK codes). This allows us to build a mapping from our `Key` enum to the correct `WORD` virtual-key code for the active layout.

### Linux (uinput) (`backend_uinput.cpp`)

On Linux we provide a uinput-based backend (implemented in `src/backend/backend_uinput.cpp`) which is compiled only when building on Linux and when the X11-based backend is not requested (the implementation is guarded with `#if defined(__linux__) && !defined(BACKEND_USE_X11)`). The uinput backend opens `/dev/uinput`, creates a virtual keyboard device, and emits `EV_KEY` events to simulate real physical key presses.

Key notes about the uinput backend:

- It performs HID-level key injection (true hardware-level simulation).
- Capabilities and behaviour:
  - `canInjectKeys`: true if `/dev/uinput` was opened successfully.
  - `canInjectText`: false — uinput injects physical keys only. Arbitrary Unicode injection would require layout-aware key sequences (not implemented).
  - `canSimulateHID`: true
  - `supportsKeyRepeat`: true
  - `needsUinputAccess`: true
- The backend tracks modifier state and sends explicit modifier press/release events (Shift/Ctrl/Alt/Super) when appropriate.
- Because the backend manipulates `/dev/uinput`, it requires appropriate permissions. Give your user access either by running as root (not recommended) or by creating a udev rule such as:

```
# /etc/udev/rules.d/99-uinput.rules
KERNEL=="uinput", MODE="0660", GROUP="input"
```

Then add your user to the `input` group (or adjust the group in the rule) and reload udev rules. After ensuring permissions, the backend will be able to create and use the virtual input device.

If you prefer an X11-based injector instead, define `BACKEND_USE_X11` when building and provide/enable an X11 backend; the uinput backend will not be compiled in that case.

In summary, uinput offers robust HID-level key simulation on Linux but requires platform permissions and does not directly support arbitrary Unicode text injection without layout mapping.

## Backend API & Behaviour

### Public API

- `BackendType type() const`  
  Returns which platform backend is in use (see `BackendType` enum).

- `Capabilities capabilities() const`  
  Returns a `Capabilities` struct describing what the backend supports (see below for field descriptions).

- `bool isReady() const`  
  Returns `true` when the backend is ready to inject events (permissions/grants are satisfied, required devices are available).

- `bool requestPermissions()`  
  Attempts to obtain or prompt for platform-level permissions where possible. Behavior varies by platform:
  - macOS: prompts for Accessibility permission and updates readiness accordingly.
  - Windows: typically returns `true`.
  - Linux (uinput): cannot obtain udev/device permissions at runtime; returns whether the backend is ready.

- Physical key events:
  - `bool keyDown(Key key)`  
    Simulates a physical key press and keeps the key logically down until `keyUp` is called. Internal modifier state is updated for modifier keys.
  - `bool keyUp(Key key)`  
    Releases a physical key that was previously pressed.
  - `bool tap(Key key)`  
    Convenience: `keyDown` → small delay → `keyUp`. Delay is configurable via `setKeyDelay()`.

- Modifier helpers:
  - `Modifier activeModifiers() const` — returns the currently tracked modifier bitmask.
  - `bool holdModifier(Modifier mods)` — presses the requested modifiers (prefers left-side variants when available).
  - `bool releaseModifier(Modifier mods)` — releases the requested modifiers if currently held.
  - `bool releaseAllModifiers()` — releases all tracked modifiers.
  - `bool combo(Modifier mods, Key key)` — holds modifiers, taps the key, then releases modifiers.

- Text input:
  - `bool typeText(const std::u32string& text)` — injects raw Unicode text (layout-independent) when the backend supports it.
  - `bool typeText(const std::string& utf8Text)` — convenience overload that converts UTF-8 to UTF-32 and calls the above.
  - `bool typeCharacter(char32_t codepoint)` — injects a single Unicode character.  
    Note: not all backends support direct Unicode injection (for example, uinput does not).

- Advanced:
  - `void flush()` — forces sync/flush of pending events (some backends buffer events).
  - `void setKeyDelay(uint32_t delayUs)` — sets the delay used by `tap`/`combo` (in microseconds).

### Capabilities explained

The returned `Capabilities` struct fields indicate backend behavior:

- `canInjectKeys` — backend can send physical key events (`keyDown`/`keyUp`/`tap`).
- `canInjectText` — backend can inject arbitrary Unicode text directly (`typeText`).
- `canSimulateHID` — true hardware-level simulation (kernel-level / driver-level events).
- `supportsKeyRepeat` — OS will generate key repeat when a key is held down; otherwise the UI layer may simulate repeats.
- `needsAccessibilityPerm` — backend requires Accessibility permissions (macOS).
- `needsInputMonitoringPerm` — backend requires Input Monitoring permission (macOS-ish workflows).
- `needsUinputAccess` — backend needs `/dev/uinput` access (Linux uinput).

### Backend type detection & selection

Use `type()` and `capabilities()` at runtime to decide how to drive input for robust behaviour (for example, prefer `tap` for simple presses, or `keyDown`/`keyUp` for long presses and combos).

### Platform-specific notes

- macOS (`backend_macos.mm`)
  - Uses layout scanning (`TISCopyCurrentKeyboardLayoutInputSource` + `UCKeyTranslate`) to map characters to `CGKeyCode`s and prefers sending mapped physical key events when possible.
  - Supports direct Unicode injection via `CGEventKeyboardSetUnicodeString` (used by `typeText`).
  - Typically requires Accessibility permission (`needsAccessibilityPerm = true`). `requestPermissions()` prompts the system dialog.

- Windows (`backend_windows.cpp`)
  - Uses `GetKeyboardLayout`, `MapVirtualKeyExW`, `ToUnicodeEx`, and `SendInput` for scancode and Unicode injection.
  - Supports both physical key events and direct Unicode (`typeText`), and simulates HID-level events via scancodes.

- Linux uinput (`backend_uinput.cpp`)
  - Creates a virtual input device via `/dev/uinput` and emits `EV_KEY` events (true HID-level).
  - Requires udev/device permissions; set up a udev rule (for example `KERNEL=="uinput", MODE="0660", GROUP="input"`) and add the user to that group so the process can open `/dev/uinput`.
  - `isReady()` returns `true` only when the device was successfully opened; `requestPermissions()` cannot obtain udev permissions at runtime.
  - Direct Unicode injection (`typeText`) is not implemented for uinput (layout-aware mapping would be required).

- Linux X11 / Wayland
  - The project now includes an X11-based OutputListener (using XInput2) for global key monitoring on X11 systems. Wayland global key monitoring is not supported by this listener (compositor APIs restrict global input monitoring). If XInput2 is not available at runtime the listener will not start. Injection backends (uinput or others) remain available for HID-level simulation and text injection where supported.

### Keys, Modifiers & Utilities

- The `Key` enum enumerates logical keys (letters, numbers, function keys, modifiers, punctuation, etc.). Use `keyToString(Key)` and `stringToKey(const std::string&)` to convert between a stable, human-readable name and the enum (used by UI and configuration).
- The `Modifier` bitmask represents held modifiers; use `hasModifier(state, flag)` to test flags. Backends generally press/release explicit physical modifier keys (e.g., `ShiftLeft`) for consistency.

### Usage notes & best practices

- Prefer `keyDown` / `keyUp` for durable presses and combos; use `tap` for simple single key presses.
- Use `combo(mods, key)` to perform typical shortcut-like operations safely: it holds modifiers, taps the key, and releases modifiers.
- When `capabilities().supportsKeyRepeat` is false, the UI layer may simulate repeats (the `core::Input` fallback exists for such cases).
- For UI display and configuration, use `keyToString`/`stringToKey` to keep labels consistent across platforms.

### Debug logging

The input backends and the OutputListener include optional debug logging that can be enabled at runtime. To enable detailed backend logs (which show whether a keystroke used a mapped physical key event or direct Unicode injection, virtual-key codes, scancodes, modifiers, layout discovery, OutputListener events, and other decision points) set the environment variable `TYPR_OSK_DEBUG_BACKEND` to a non-empty value (for example `TYPR_OSK_DEBUG_BACKEND=1`) before launching the application. These logs are intentionally off by default to avoid noisy output during normal runs.

## Advantages of This Approach

1. **Layout Agnostic**: Works out-of-the-box with QWERTY, AZERTY, QWERTZ, Dvorak, Colemak, etc.
2. **Native Shortcuts**: By sending real virtual key codes rather than just strings, system-wide shortcuts (like Cmd+C / Ctrl+C) continue to work correctly regardless of the layout.
3. **No Hardcoded Tables**: We don't need to maintain massive, error-prone tables for every possible keyboard layout in the world.

## Feedback

If you experience issues with specific keyboard layouts—such as characters not appearing, incorrect symbols being typed, or modifiers behaving unexpectedly **please do not hesitate to send feedback**.
