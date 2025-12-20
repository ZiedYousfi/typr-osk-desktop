#pragma once

#include "../backend/backend.hpp"
#include "../ui/widgets.hpp"

#include <QAction>
#include <QDebug>
#include <QString>
#include <functional>

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

private:
  /**
   * @brief Internal handler for button trigger events (clicks)
   */
  void onTriggered();

  backend::Key key_;
  ui::Widget::RightClickableToolButton *button_;
  backend::InputBackend *backend_;
  QAction *action_;
  backend::Mod mods_{backend::Mod_None};
  bool isToggleMode_{false};
  bool isToggled_{false};
  KeyCallback onKeyPressed_;
  KeyCallback onKeyReleased_;
};

} // namespace core
