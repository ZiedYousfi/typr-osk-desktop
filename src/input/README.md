# Input Subsystem

This directory contains the cross-platform input injection backend for Typr OSK. It provides a unified interface to simulate keyboard events across different operating systems.

## Key Handling Philosophy

The "right way" to implement a virtual keyboard is to send low-level hardware scan codes or virtual key codes. This allows the Operating System to handle the translation from a physical key press to a specific character based on the user's active keyboard layout (e.g., QWERTY, AZERTY, Dvorak). This ensures that the virtual keyboard behaves exactly like a physical one.

## macOS Implementation & Workarounds

On macOS (`input_macos.mm`), using the standard Quartz virtual key codes (like `kVK_ANSI_A`) proved problematic because these codes are hardcoded to physical positions on a US-QWERTY layout. If a user changes their keyboard layout in macOS System Settings, the mapping between these virtual codes and the resulting characters breaks.

### The Unicode Workaround

To resolve the mismatch where the OSK would type the wrong characters on non-QWERTY layouts, we have implemented a **workaround**:

For printable characters (letters, numbers, and punctuation), the macOS backend bypasses positional key codes and instead injects Unicode characters directly using `CGEventKeyboardSetUnicodeString`.

**Note:** This is considered a "bad" workaround because:
1. It bypasses the OS's native layout translation logic.
2. It requires the backend to manually calculate shifted characters (e.g., knowing that `Shift + 1` is `!`).
3. It may not support complex layouts with dead keys, unique ligatures, or specialized input methods correctly.

## Future Changes & Feedback

This implementation is subject to change. We are looking for a more robust way to handle layout-independent virtual key codes on macOS that doesn't rely on high-level string injection.

If you experience issues with specific keyboard layouts—such as characters not appearing, incorrect symbols being typed, or modifiers behaving unexpectedly—**please do not hesitate to send feedback**. Your input helps us refine these workarounds for a better cross-layout experience.
