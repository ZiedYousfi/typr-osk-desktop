#include "input.hpp"

#include <QAction>
#include <QDebug>
#include <QString>
#include <QTimer>

namespace core {

Input::Input(backend::Key key, ui::Widget::RightClickableToolButton *button,
             backend::InputBackend *backend)
    : key_(key), button_(button), backend_(backend),
      action_(new QAction(QString::fromStdString(backend::keyToString(key)),
                          button)) {
  // Configure the action (toggle state is set later by setToggleMode)
  action_->setCheckable(false);

  // QAction::triggered is connected only for toggle mode (via setToggleMode())
  // so programmatic triggers/shortcuts for toggles still work without causing
  // duplicate handling for normal mouse clicks.

  // Connect to button press/release so we can implement press-and-hold /
  // auto-repeat behaviour (start sending input before release if the user
  // keeps the button pressed).
  pressedConnection_ = QObject::connect(button_, &QToolButton::pressed, button_,
                                        [this]() { onPressed(); });

  releasedConnection_ = QObject::connect(button_, &QToolButton::released,
                                         button_, [this]() { onReleased(); });

  // Create timers and parent them to the button (so they follow its lifetime)
  holdTimer_ = new QTimer(button_);
  holdTimer_->setSingleShot(true);
  holdTimerConnection_ = QObject::connect(holdTimer_, &QTimer::timeout, button_,
                                          [this]() { onHoldTimeout(); });

  // Fallback repeat timer: if the backend cannot act as a HID and doesn't
  // generate native autorepeats for an injected keyDown, we will simulate
  // repeated key presses (tap) while the key is held.
  repeatTimer_ = new QTimer(button_);
  repeatTimer_->setSingleShot(false);
  repeatTimer_->setInterval(repeatIntervalMs_);
  repeatTimerConnection_ =
      QObject::connect(repeatTimer_, &QTimer::timeout, button_, [this]() {
        // Simulate a repeated key press (tap). We call the callbacks the same
        // way a physical repeat would appear to the rest of the system.
        if (tap()) {
          if (onKeyPressed_) {
            onKeyPressed_(key_);
          }
          if (onKeyReleased_) {
            onKeyReleased_(key_);
          }
        }
      });

  // Set up the button
  button_->setDefaultAction(action_);
  button_->setToolButtonStyle(Qt::ToolButtonTextOnly);

  qDebug() << "[Core::Input] Created input for key:"
           << QString::fromStdString(backend::keyToString(key));
}

Input::Input(Input &&other) noexcept
    : key_(other.key_), button_(other.button_), backend_(other.backend_),
      action_(other.action_), isToggleMode_(other.isToggleMode_),
      isToggled_(other.isToggled_),
      onKeyPressed_(std::move(other.onKeyPressed_)),
      onKeyReleased_(std::move(other.onKeyReleased_)),
      holdThresholdMs_(other.holdThresholdMs_), isPressed_(other.isPressed_),
      isHeld_(other.isHeld_), holdTimer_(other.holdTimer_) {
  // Rewire connections: disconnect any connections that referenced the old
  // object's lambdas and reconnect them to this instance.
  if (other.pressedConnection_ != nullptr) {
    QObject::disconnect(other.pressedConnection_);
  }
  if (button_ != nullptr) {
    pressedConnection_ = QObject::connect(button_, &QToolButton::pressed,
                                          button_, [this]() { onPressed(); });
  }

  if (other.releasedConnection_ != nullptr) {
    QObject::disconnect(other.releasedConnection_);
  }
  if (button_ != nullptr) {
    releasedConnection_ = QObject::connect(button_, &QToolButton::released,
                                           button_, [this]() { onReleased(); });
  }

  if (other.actionToggledConnection_ != nullptr) {
    QObject::disconnect(other.actionToggledConnection_);
  }
  if ((action_ != nullptr) && (other.actionToggledConnection_ != nullptr)) {
    actionToggledConnection_ =
        QObject::connect(action_, &QAction::toggled, action_,
                         [this](bool checked) { onToggled(checked); });
  }

  // Reattach timer handler to use this instance
  if (other.holdTimer_ != nullptr) {
    if (other.holdTimerConnection_ != nullptr) {
      QObject::disconnect(other.holdTimerConnection_);
    }
    if (holdTimer_ != nullptr) {
      holdTimerConnection_ = QObject::connect(
          holdTimer_, &QTimer::timeout, button_, [this]() { onHoldTimeout(); });
    }
  }

  // Reattach fallback repeat timer (if present) so a moved Input continues to
  // simulate repeated taps when the backend doesn't generate native repeats.
  if (other.repeatTimer_ != nullptr) {
    if (other.repeatTimerConnection_ != nullptr) {
      QObject::disconnect(other.repeatTimerConnection_);
    }
    if (repeatTimer_ == nullptr) {
      repeatTimerConnection_ =
          QObject::connect(repeatTimer_, &QTimer::timeout, button_, [this]() {
            if (tap()) {
              if (onKeyPressed_) {
                onKeyPressed_(key_);
              }
              if (onKeyReleased_) {
                onKeyReleased_(key_);
              }
            }
          });
    }
  }

  // Clear the other's raw pointers so its destructor won't touch them
  other.button_ = nullptr;
  other.action_ = nullptr;
  other.backend_ = nullptr;
  other.holdTimer_ = nullptr;
  other.pressedConnection_ = QMetaObject::Connection();
  other.releasedConnection_ = QMetaObject::Connection();
  other.actionToggledConnection_ = QMetaObject::Connection();
  other.holdTimerConnection_ = QMetaObject::Connection();
}

Input &Input::operator=(Input &&other) noexcept {
  if (this != &other) {
    // Tear down current connections/timers cleanly
    tearDownConnections();

    // Move simple state
    key_ = other.key_;
    button_ = other.button_;
    backend_ = other.backend_;
    action_ = other.action_;
    isToggleMode_ = other.isToggleMode_;
    isToggled_ = other.isToggled_;
    onKeyPressed_ = std::move(other.onKeyPressed_);
    onKeyReleased_ = std::move(other.onKeyReleased_);
    holdThresholdMs_ = other.holdThresholdMs_;
    isPressed_ = other.isPressed_;
    isHeld_ = other.isHeld_;
    holdTimer_ = other.holdTimer_;

    // Rewire new connections as in the move ctor
    if (other.pressedConnection_ != nullptr) {
      QObject::disconnect(other.pressedConnection_);
    }
    if (button_ != nullptr) {
      pressedConnection_ = QObject::connect(button_, &QToolButton::pressed,
                                            button_, [this]() { onPressed(); });
    }

    if (other.releasedConnection_ != nullptr) {
      QObject::disconnect(other.releasedConnection_);
    }
    if (button_ != nullptr) {
      releasedConnection_ = QObject::connect(
          button_, &QToolButton::released, button_, [this]() { onReleased(); });
    }

    if (other.actionToggledConnection_ != nullptr) {
      QObject::disconnect(other.actionToggledConnection_);
    }
    if ((action_ != nullptr) && (other.actionToggledConnection_ != nullptr)) {
      actionToggledConnection_ =
          QObject::connect(action_, &QAction::toggled, action_,
                           [this](bool checked) { onToggled(checked); });
    }

    if (other.holdTimer_ != nullptr) {
      if (other.holdTimerConnection_ != nullptr) {
        QObject::disconnect(other.holdTimerConnection_);
      }
      if (holdTimer_ != nullptr) {
        holdTimerConnection_ =
            QObject::connect(holdTimer_, &QTimer::timeout, button_,
                             [this]() { onHoldTimeout(); });
      }
    }

    // Clear other's pointers
    other.button_ = nullptr;
    other.action_ = nullptr;
    other.backend_ = nullptr;
    other.holdTimer_ = nullptr;
    other.pressedConnection_ = QMetaObject::Connection();
    other.releasedConnection_ = QMetaObject::Connection();
    other.actionToggledConnection_ = QMetaObject::Connection();
    other.holdTimerConnection_ = QMetaObject::Connection();
  }
  return *this;
}

Input::~Input() { tearDownConnections(); }

backend::Key Input::key() const { return key_; }

ui::Widget::RightClickableToolButton *Input::button() const { return button_; }

bool Input::isToggleMode() const { return isToggleMode_; }

bool Input::isToggled() const { return isToggled_; }

void Input::setToggleMode(bool toggle) {
  isToggleMode_ = toggle;
  action_->setCheckable(toggle);

  // Only connect QAction::toggled for toggle-mode keys so programmatic
  // triggers (shortcuts) and clicks both toggle via the same handler.
  if (toggle) {
    if (actionToggledConnection_ == nullptr) {
      actionToggledConnection_ =
          QObject::connect(action_, &QAction::toggled, action_,
                           [this](bool checked) { onToggled(checked); });
    }
  } else {
    if (actionToggledConnection_ != nullptr) {
      QObject::disconnect(actionToggledConnection_);
      actionToggledConnection_ = QMetaObject::Connection();
    }
  }

  // Keep action checked state consistent with the logical toggle
  action_->setChecked(isToggled_);
}

void Input::setOnKeyPressed(KeyCallback callback) {
  onKeyPressed_ = std::move(callback);
}

void Input::setOnKeyReleased(KeyCallback callback) {
  onKeyReleased_ = std::move(callback);
}

void Input::keyDown() {
  if (button_ != nullptr) {
    button_->setDown(true);
  }
}

void Input::keyUp() {
  if (button_ != nullptr) {
    button_->setDown(false);
  }
}

bool Input::tap() {
  if (backend_ == nullptr || !backend_->isReady()) {
    qDebug() << "[core::Input] Backend not ready for key:"
             << QString::fromStdString(backend::keyToString(key_));
    return false;
  }

  qDebug() << "[core::Input] Tapping key:"
           << QString::fromStdString(backend::keyToString(key_));
  backend::KeyStroke ks{key_};
  return backend_->tap(ks);
}

bool Input::pressDown() {
  if (backend_ == nullptr || !backend_->isReady()) {
    return false;
  }

  qDebug() << "[core::Input] Key down:"
           << QString::fromStdString(backend::keyToString(key_));
  backend::KeyStroke ks{key_};
  return backend_->keyDown(ks);
}

bool Input::pressUp() {
  if (backend_ == nullptr || !backend_->isReady()) {
    return false;
  }

  qDebug() << "[core::Input] Key up:"
           << QString::fromStdString(backend::keyToString(key_));
  backend::KeyStroke ks{key_};
  return backend_->keyUp(ks);
}

void Input::setHoldThresholdMs(int thresholdInMs) {
  holdThresholdMs_ = thresholdInMs;
  if (holdTimer_ != nullptr) {
    holdTimer_->setInterval(thresholdInMs);
  }
}

int Input::holdThresholdMs() const { return holdThresholdMs_; }

// Manual repeat APIs removed; per-key hold threshold controls keyDown timing

void Input::onPressed() {
  if (button_ == nullptr) {
    return;
  }

  isPressed_ = true;
  isHeld_ = false;
  // Visual feedback
  button_->setDown(true);

  // For toggle keys, do not send keyDown on press; QAction::toggled will handle
  // toggle behavior on release. For normal keys, wait for threshold before
  // sending keyDown; zero threshold means immediate keyDown on press.
  if (!isToggleMode_) {
    if (holdThresholdMs_ > 0 && (holdTimer_ != nullptr)) {
      holdTimer_->start(holdThresholdMs_);
    } else {
      // zero threshold -> immediate keyDown
      if (pressDown()) {
        if (onKeyPressed_) {
          onKeyPressed_(key_);
        }
        isHeld_ = true;
      }
    }
  }
}

void Input::onHoldTimeout() {
  // When hold threshold elapses, send a single keyDown for non-toggle keys.
  if (!isPressed_ || isToggleMode_ || (holdTimer_ == nullptr)) {
    return;
  }

  if (pressDown()) {
    if (onKeyPressed_) {
      onKeyPressed_(key_);
    }
    isHeld_ = true;
  }
}

void Input::onReleased() {
  if (button_ == nullptr) {
    return;
  }

  // Stop pending hold detection
  if ((holdTimer_ != nullptr) && holdTimer_->isActive()) {
    holdTimer_->stop();
  }

  bool wasHeld = isHeld_;
  isPressed_ = false;
  isHeld_ = false;

  // Reset visual state
  button_->setDown(false);

  if (isToggleMode_) {
    // Toggle behavior is handled by QAction::toggled (onToggled).
    return;
  }

  if (wasHeld) {
    // We sent a keyDown when threshold passed; now release it.
    pressUp();
    if (onKeyReleased_) {
      onKeyReleased_(key_);
    }
  } else {
    // Short press -> tap
    tap();
    if (onKeyPressed_) {
      onKeyPressed_(key_);
    }
    if (onKeyReleased_) {
      onKeyReleased_(key_);
    }
  }
}

void Input::onToggled(bool checked) {
  if (checked) {
    pressDown();
    isToggled_ = true;
    action_->setChecked(true);
    button_->setDown(true);
    if (onKeyPressed_) {
      onKeyPressed_(key_);
    }
  } else {
    pressUp();
    isToggled_ = false;
    action_->setChecked(false);
    button_->setDown(false);
    if (onKeyReleased_) {
      onKeyReleased_(key_);
    }
  }
}

void Input::tearDownConnections() {
  // Stop threshold detection timer
  if (holdTimer_ != nullptr) {
    holdTimer_->stop();
    if (holdTimerConnection_ != nullptr) {
      QObject::disconnect(holdTimerConnection_);
      holdTimerConnection_ = QMetaObject::Connection();
    }
    holdTimer_ = nullptr;
  }

  // Stop fallback repeat timer as well
  if (repeatTimer_ != nullptr) {
    repeatTimer_->stop();
    if (repeatTimerConnection_ != nullptr) {
      QObject::disconnect(repeatTimerConnection_);
      repeatTimerConnection_ = QMetaObject::Connection();
    }
    repeatTimer_ = nullptr;
  }

  if (pressedConnection_ != nullptr) {
    QObject::disconnect(pressedConnection_);
    pressedConnection_ = QMetaObject::Connection();
  }
  if (releasedConnection_ != nullptr) {
    QObject::disconnect(releasedConnection_);
    releasedConnection_ = QMetaObject::Connection();
  }
  if (actionToggledConnection_ != nullptr) {
    QObject::disconnect(actionToggledConnection_);
    actionToggledConnection_ = QMetaObject::Connection();
  }
}

} // namespace core
