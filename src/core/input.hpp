// core/input.hpp
#pragma once

#include "../input/input.hpp"
#include "../ui/ui.hpp"

#include <QAction>
#include <QDebug>
#include <QString>
#include <functional>

namespace Core {

// Input class that connects a Key to a RightClickableToolButton
// and performs the actual key injection when the button is clicked
class Input {
public:
  // Callback types for key events
  using KeyCallback = std::function<void(input::Key)>;

  // Constructor that takes the key, button, and a reference to the input
  // backend
  Input(input::Key key, Ui::Widget::RightClickableToolButton *button,
        input::InputBackend *backend)
      : key_(key), button_(button), backend_(backend),
        action_(new QAction(QString::fromStdString(input::keyToString(key)),
                            button)) {

    // Configure the action
    action_->setCheckable(false);

    // Connect button click to the tap action
    QObject::connect(action_, &QAction::triggered, [this]() { onTriggered(); });

    // Set up the button
    button_->setDefaultAction(action_);
    button_->setToolButtonStyle(Qt::ToolButtonTextOnly);

    qDebug() << "[Core::Input] Created input for key:"
             << QString::fromStdString(input::keyToString(key));
  }

  // Disable copy
  Input(const Input &) = delete;
  Input &operator=(const Input &) = delete;

  // Enable move
  Input(Input &&other) noexcept
      : key_(other.key_), button_(other.button_), backend_(other.backend_),
        action_(other.action_), mods_(other.mods_),
        isToggleMode_(other.isToggleMode_), isToggled_(other.isToggled_),
        onKeyPressed_(std::move(other.onKeyPressed_)),
        onKeyReleased_(std::move(other.onKeyReleased_)) {
    other.button_ = nullptr;
    other.action_ = nullptr;
    other.backend_ = nullptr;
  }

  Input &operator=(Input &&other) noexcept {
    if (this != &other) {
      key_ = other.key_;
      button_ = other.button_;
      backend_ = other.backend_;
      action_ = other.action_;
      mods_ = other.mods_;
      isToggleMode_ = other.isToggleMode_;
      isToggled_ = other.isToggled_;
      onKeyPressed_ = std::move(other.onKeyPressed_);
      onKeyReleased_ = std::move(other.onKeyReleased_);
      other.button_ = nullptr;
      other.action_ = nullptr;
      other.backend_ = nullptr;
    }
    return *this;
  }

  ~Input() = default;

  // Getters
  [[nodiscard]] input::Key key() const { return key_; }
  [[nodiscard]] Ui::Widget::RightClickableToolButton *button() const {
    return button_;
  }
  [[nodiscard]] input::Mod modifiers() const { return mods_; }
  [[nodiscard]] bool isToggleMode() const { return isToggleMode_; }
  [[nodiscard]] bool isToggled() const { return isToggled_; }

  // Set modifiers to apply with this key
  void setModifiers(input::Mod mods) { mods_ = mods; }

  // Set toggle mode - when true, the key will be held down until clicked again
  void setToggleMode(bool toggle) {
    isToggleMode_ = toggle;
    action_->setCheckable(toggle);
  }

  // Set callback for key press events
  void setOnKeyPressed(KeyCallback callback) {
    onKeyPressed_ = std::move(callback);
  }

  // Set callback for key release events
  void setOnKeyReleased(KeyCallback callback) {
    onKeyReleased_ = std::move(callback);
  }

  // Simulate key down (visual feedback only)
  void keyDown() { button_->setDown(true); }

  // Simulate key up (visual feedback only)
  void keyUp() { button_->setDown(false); }

  // Perform a key tap (press and release)
  bool tap() {
    if (backend_ == nullptr || !backend_->isReady()) {
      qDebug() << "[Core::Input] Backend not ready for key:"
               << QString::fromStdString(input::keyToString(key_));
      return false;
    }

    qDebug() << "[Core::Input] Tapping key:"
             << QString::fromStdString(input::keyToString(key_));
    return backend_->tap(key_, mods_);
  }

  // Press the key down (and hold)
  bool pressDown() {
    if (backend_ == nullptr || !backend_->isReady()) {
      return false;
    }

    qDebug() << "[Core::Input] Key down:"
             << QString::fromStdString(input::keyToString(key_));
    return backend_->keyDown(key_, mods_);
  }

  // Release the key
  bool pressUp() {
    if (backend_ == nullptr || !backend_->isReady()) {
      return false;
    }

    qDebug() << "[Core::Input] Key up:"
             << QString::fromStdString(input::keyToString(key_));
    return backend_->keyUp(key_, mods_);
  }

private:
  void onTriggered() {
    if (isToggleMode_) {
      // Toggle mode: press down or release
      if (isToggled_) {
        pressUp();
        isToggled_ = false;
        button_->setDown(false);
        if (onKeyReleased_) {
          onKeyReleased_(key_);
        }
      } else {
        pressDown();
        isToggled_ = true;
        button_->setDown(true);
        if (onKeyPressed_) {
          onKeyPressed_(key_);
        }
      }
    } else {
      // Normal mode: tap (press and release)
      tap();
      if (onKeyPressed_) {
        onKeyPressed_(key_);
      }
      if (onKeyReleased_) {
        onKeyReleased_(key_);
      }
    }
  }

  input::Key key_;
  Ui::Widget::RightClickableToolButton *button_;
  input::InputBackend *backend_;
  QAction *action_;
  input::Mod mods_{input::Mod_None};
  bool isToggleMode_{false};
  bool isToggled_{false};
  KeyCallback onKeyPressed_;
  KeyCallback onKeyReleased_;
};

} // namespace Core
