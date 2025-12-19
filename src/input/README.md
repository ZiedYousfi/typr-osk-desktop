# Input Subsystem

This directory contains the cross-platform input injection backend for Typr OSK. It provides a unified interface to simulate keyboard events across different operating systems.

## Key Handling Philosophy

The "right way" to implement a virtual keyboard is to send low-level hardware scan codes or virtual key codes. This allows the Operating System to handle the translation from a physical key press to a specific character based on the user's active keyboard layout (e.g., QWERTY, AZERTY, Dvorak). This ensures that the virtual keyboard behaves exactly like a physical one.

## Dynamic Layout Mapping

Instead of hardcoding the relationship between our internal `Key` enum and platform-specific virtual key codes (which are often tied to physical positions like US-QWERTY), we use a **dynamic scanning approach**.

### How it works:

1. **Initialization**: When the `InputBackend` is created, it queries the current system keyboard layout.
2. **Scanning**: The backend iterates through all available virtual key codes and asks the OS: _"If this physical key were pressed, what character would it produce?"_
3. **Mapping**: We then build a reverse lookup table that maps our logical keys (like `Key::A`) to the specific physical key code that produces that character in the user's current layout.

This approach ensures that if a user is using an AZERTY layout, `Key::A` will automatically map to the physical key that produces 'A' on their keyboard, rather than the physical position where 'A' is located on a QWERTY keyboard.

## Platform Implementation Details

### macOS (`input_macos.mm`)

We use `TISCopyCurrentKeyboardLayoutInputSource` and `UCKeyTranslate` to perform the layout scanning. This allows us to discover the `CGKeyCode` for every character. For complex inputs or cases where translation is preferred, we still support direct Unicode injection via `CGEventKeyboardSetUnicodeString`, but we prioritize physical key events to preserve native OS behavior (like keyboard shortcuts).

### Windows (`input_windows.cpp`)

We use `GetKeyboardLayout`, `MapVirtualKeyExW`, and `ToUnicodeEx` to scan the virtual key space (VK codes). This allows us to build a mapping from our `Key` enum to the correct `WORD` virtual-key code for the active layout.

## Advantages of This Approach

1. **Layout Agnostic**: Works out-of-the-box with QWERTY, AZERTY, QWERTZ, Dvorak, Colemak, etc.
2. **Native Shortcuts**: By sending real virtual key codes rather than just strings, system-wide shortcuts (like Cmd+C / Ctrl+C) continue to work correctly regardless of the layout.
3. **No Hardcoded Tables**: We don't need to maintain massive, error-prone tables for every possible keyboard layout in the world.

## Feedback

If you experience issues with specific keyboard layouts—such as characters not appearing, incorrect symbols being typed, or modifiers behaving unexpectedly **please do not hesitate to send feedback**.
