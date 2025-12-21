#pragma once

#include "../backend/backend.hpp"
#include "../ui/widgets.hpp"

#include <QAction>
#include <QDebug>
#include <QObject>
#include <QString>
#include <QTimer>
#include <functional>

#define DEFAULT_HOLD_THRESHOLD 300
#define DEFAULT_REPEAT_INTERVAL 80

class QAction;

namespace core {

/**
 * @brief Input class that connects a Key to a RightClickableToolButton
 * and performs the actual key injection when the button is clicked
 */
class Input {
public:
  // Callback types for key events
  using KeyCallback = std::function<void(backend::Key)>;

  // Constructor that takes the key, button, and a reference to the input
  // backend
  Input(backend::Key key, ui::Widget::RightClickableToolButton *button,
        backend::InputBackend *backend);

  // Disable copy
  Input(const Input &) = delete;
  Input &operator=(const Input &) = delete;

  // Enable move
  Input(Input &&other) noexcept;
  Input &operator=(Input &&other) noexcept;

  ~Input();

  // Getters
  [[nodiscard]] backend::Key key() const;
  [[nodiscard]] ui::Widget::RightClickableToolButton *button() const;
  [[nodiscard]] backend::Mod modifiers() const;
  [[nodiscard]] bool isToggleMode() const;
  [[nodiscard]] bool isToggled() const;

  // Set modifiers to apply with this key
  void setModifiers(backend::Mod mods);

  // Set toggle mode - when true, the key will be held down until clicked again
  void setToggleMode(bool toggle);

  // Set callback for key press events
  void setOnKeyPressed(KeyCallback callback);

  // Set callback for key release events
  void setOnKeyReleased(KeyCallback callback);

  // Simulate key down (visual feedback only)
  void keyDown();

  // Simulate key up (visual feedback only)
  void keyUp();

  // Perform a key tap (press and release)
  bool tap();

  // Press the key down (and hold)
  bool pressDown();

  // Release the key
  bool pressUp();

  // -----------------------
  // Per-key hold threshold
  // -----------------------

  /**
   * @brief Set how many milliseconds the user must hold the button before we
   * send a keyDown for the key. If the user releases before this threshold the
   * key will be treated as a tap.
   *
   * Example: a default of 300 ms means short presses are interpreted as taps
   * (keyDown+keyUp on release), while holding longer than 300 ms will send a
   * single keyDown when the threshold is crossed and a keyUp on release.
   */
  void setHoldThresholdMs(int thresholdInMs);
  [[nodiscard]] int holdThresholdMs() const;

private:
  /**
   * @brief Internal handler for button trigger events (clicks)
   *
   * Note: by default Input uses native hold semantics (keyDown on press /
   * keyUp on release). The class listens for press/release to support
   * native-hold behaviour (so the OS can manage repeat). The
   * QAction::triggered signal is only connected for toggle-mode or for
   * explicit programmatic triggers (e.g., shortcuts) and is not used for
   * normal mouse click handling to avoid duplicate event handling.
   */
  void onTriggered();

  // Internal handlers for press / release lifecycle
  void onPressed();
  void onReleased();
  void onToggled(bool checked); // QAction toggled handler (toggle-mode keys)

  // Called when the hold threshold has been reached (start repeating)
  void onHoldTimeout();

  // (manual auto-repeat removed; hold threshold now controls when keyDown is
  // sent)

  // Helper to disconnect and stop timers/connections. Used during move and
  // destruction.
  void tearDownConnections();

  backend::Key key_;
  ui::Widget::RightClickableToolButton *button_;
  backend::InputBackend *backend_;
  QAction *action_;
  backend::Mod mods_{backend::Mod_None};
  bool isToggleMode_{false};
  bool isToggled_{false};
  KeyCallback onKeyPressed_;
  KeyCallback onKeyReleased_;

  // Hold threshold configuration and transient state
  // Default is 300 ms before sending keyDown; short presses are taps.
  int holdThresholdMs_{
      DEFAULT_HOLD_THRESHOLD}; // milliseconds; threshold before sending keyDown
                               // (default: 300 ms)

  // Transient state
  bool isPressed_{false}; // true while mouse button is down
  bool isHeld_{
      false}; // true when holdThreshold has elapsed and keyDown has been sent

  // Timer used to detect the hold threshold; parented to button_
  QTimer *holdTimer_{nullptr};

  // Fallback repeat timer: used only if the backend does not provide native
  // autorepeat when a physical-like keyDown is injected. This is private and
  // controlled internally; default repeat interval is 80 ms.
  QTimer *repeatTimer_{nullptr};
  QMetaObject::Connection repeatTimerConnection_;
  int repeatIntervalMs_{DEFAULT_REPEAT_INTERVAL};

  // Stored connections so move / reattachment can disconnect/reconnect safely
  QMetaObject::Connection pressedConnection_;
  QMetaObject::Connection releasedConnection_;
  QMetaObject::Connection actionToggledConnection_;
  QMetaObject::Connection holdTimerConnection_;
};

} // namespace core
