#include "input.hpp"

#include <QAction>
#include <QDebug>
#include <QString>

namespace Core {

Input::Input(input::Key key, Ui::Widget::RightClickableToolButton *button,
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

Input::Input(Input &&other) noexcept
    : key_(other.key_), button_(other.button_), backend_(other.backend_),
      action_(other.action_), mods_(other.mods_),
      isToggleMode_(other.isToggleMode_), isToggled_(other.isToggled_),
      onKeyPressed_(std::move(other.onKeyPressed_)),
      onKeyReleased_(std::move(other.onKeyReleased_)) {
  other.button_ = nullptr;
  other.action_ = nullptr;
  other.backend_ = nullptr;
}

Input &Input::operator=(Input &&other) noexcept {
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

Input::~Input() = default;

input::Key Input::key() const { return key_; }

Ui::Widget::RightClickableToolButton *Input::button() const { return button_; }

input::Mod Input::modifiers() const { return mods_; }

bool Input::isToggleMode() const { return isToggleMode_; }

bool Input::isToggled() const { return isToggled_; }

void Input::setModifiers(input::Mod mods) { mods_ = mods; }

void Input::setToggleMode(bool toggle) {
  isToggleMode_ = toggle;
  action_->setCheckable(toggle);
}

void Input::setOnKeyPressed(KeyCallback callback) {
  onKeyPressed_ = std::move(callback);
}

void Input::setOnKeyReleased(KeyCallback callback) {
  onKeyReleased_ = std::move(callback);
}

void Input::keyDown() { button_->setDown(true); }

void Input::keyUp() { button_->setDown(false); }

bool Input::tap() {
  if (backend_ == nullptr || !backend_->isReady()) {
    qDebug() << "[Core::Input] Backend not ready for key:"
             << QString::fromStdString(input::keyToString(key_));
    return false;
  }

  qDebug() << "[Core::Input] Tapping key:"
           << QString::fromStdString(input::keyToString(key_));
  return backend_->tap(key_, mods_);
}

bool Input::pressDown() {
  if (backend_ == nullptr || !backend_->isReady()) {
    return false;
  }

  qDebug() << "[Core::Input] Key down:"
           << QString::fromStdString(input::keyToString(key_));
  return backend_->keyDown(key_, mods_);
}

bool Input::pressUp() {
  if (backend_ == nullptr || !backend_->isReady()) {
    return false;
  }

  qDebug() << "[Core::Input] Key up:"
           << QString::fromStdString(input::keyToString(key_));
  return backend_->keyUp(key_, mods_);
}

void Input::onTriggered() {
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

} // namespace Core
