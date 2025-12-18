#include <QAction>
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>
#include <vector>

#include "core/input.hpp"
#include "input/input.hpp"
#include "ui/ui.hpp"

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  qDebug() << "[main] Application started";

  // Install event filter to block activation events at the Qt level
  Ui::installNoActivationFilter(&app);

  // Create the input backend
  input::InputBackend keyboard;

  if (!keyboard.isReady()) {
    keyboard.requestPermissions(); // Shows macOS dialog
  }

  // Create main window widget
  QWidget window;
  window.setWindowTitle("Typr OSK");

  window.setWindowFlags(Qt::WindowStaysOnTopHint |
                        Qt::WindowDoesNotAcceptFocus | Qt::Tool);

  window.setAttribute(Qt::WA_ShowWithoutActivating);
  window.setAttribute(Qt::WA_TranslucentBackground);

  // Main vertical layout for keyboard rows
  auto *mainLayout = new QVBoxLayout(&window);
  mainLayout->setContentsMargins(4, 4, 4, 4);
  mainLayout->setSpacing(4);

  // Store Core::Input objects to keep them alive
  std::vector<std::unique_ptr<Core::Input>> keyboardInputs;

  // Helper to add keys to a row
  auto addKey = [&](input::Key key, QHBoxLayout *row, int width = 40,
                    bool toggle = false) {
    auto *btn = new Ui::Widget::RightClickableToolButton(&window);
    btn->setMinimumWidth(width);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto input = std::make_unique<Core::Input>(key, btn, &keyboard);
    if (toggle) {
      input->setToggleMode(true);
    }

    row->addWidget(btn);
    keyboardInputs.push_back(std::move(input));
  };

  // --- Row 1: Numbers & Backspace ---
  QHBoxLayout *row1 = new QHBoxLayout();
  row1->setSpacing(4);
  addKey(input::Key::Escape, row1);
  addKey(input::Key::Grave, row1);
  addKey(input::Key::Num1, row1);
  addKey(input::Key::Num2, row1);
  addKey(input::Key::Num3, row1);
  addKey(input::Key::Num4, row1);
  addKey(input::Key::Num5, row1);
  addKey(input::Key::Num6, row1);
  addKey(input::Key::Num7, row1);
  addKey(input::Key::Num8, row1);
  addKey(input::Key::Num9, row1);
  addKey(input::Key::Num0, row1);
  addKey(input::Key::Minus, row1);
  addKey(input::Key::Equal, row1);
  addKey(input::Key::Backspace, row1, 80);
  mainLayout->addLayout(row1);

  // --- Row 2: Tab & QWERTY ---
  QHBoxLayout *row2 = new QHBoxLayout();
  row2->setSpacing(4);
  addKey(input::Key::Tab, row2, 60);
  addKey(input::Key::Q, row2);
  addKey(input::Key::W, row2);
  addKey(input::Key::E, row2);
  addKey(input::Key::R, row2);
  addKey(input::Key::T, row2);
  addKey(input::Key::Y, row2);
  addKey(input::Key::U, row2);
  addKey(input::Key::I, row2);
  addKey(input::Key::O, row2);
  addKey(input::Key::P, row2);
  addKey(input::Key::LeftBracket, row2);
  addKey(input::Key::RightBracket, row2);
  addKey(input::Key::Backslash, row2, 60);
  mainLayout->addLayout(row2);

  // --- Row 3: Caps & ASDF ---
  QHBoxLayout *row3 = new QHBoxLayout();
  row3->setSpacing(4);
  addKey(input::Key::CapsLock, row3, 70, true);
  addKey(input::Key::A, row3);
  addKey(input::Key::S, row3);
  addKey(input::Key::D, row3);
  addKey(input::Key::F, row3);
  addKey(input::Key::G, row3);
  addKey(input::Key::H, row3);
  addKey(input::Key::J, row3);
  addKey(input::Key::K, row3);
  addKey(input::Key::L, row3);
  addKey(input::Key::Semicolon, row3);
  addKey(input::Key::Apostrophe, row3);
  addKey(input::Key::Enter, row3, 90);
  mainLayout->addLayout(row3);

  // --- Row 4: Shift & ZXCV ---
  QHBoxLayout *row4 = new QHBoxLayout();
  row4->setSpacing(4);
  addKey(input::Key::ShiftLeft, row4, 100, true);
  addKey(input::Key::Z, row4);
  addKey(input::Key::X, row4);
  addKey(input::Key::C, row4);
  addKey(input::Key::V, row4);
  addKey(input::Key::B, row4);
  addKey(input::Key::N, row4);
  addKey(input::Key::M, row4);
  addKey(input::Key::Comma, row4);
  addKey(input::Key::Period, row4);
  addKey(input::Key::Slash, row4);
  addKey(input::Key::ShiftRight, row4, 100, true);
  mainLayout->addLayout(row4);

  // --- Row 5: Modifiers & Space ---
  QHBoxLayout *row5 = new QHBoxLayout();
  row5->setSpacing(4);
  addKey(input::Key::CtrlLeft, row5, 60, true);
  addKey(input::Key::AltLeft, row5, 60, true);
  addKey(input::Key::SuperLeft, row5, 60, true);
  addKey(input::Key::Space, row5, 250);
  addKey(input::Key::SuperRight, row5, 60, true);
  addKey(input::Key::AltRight, row5, 60, true);

  // Arrow Keys
  addKey(input::Key::Left, row5);
  addKey(input::Key::Up, row5);
  addKey(input::Key::Down, row5);
  addKey(input::Key::Right, row5);

  // Add a Quit button at the end
  auto *quitBtn = new Ui::Widget::RightClickableToolButton(&window);
  quitBtn->setText("Quit");
  quitBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
  quitBtn->setMinimumWidth(60);
  QObject::connect(quitBtn, &QToolButton::clicked, [&]() { app.quit(); });
  row5->addWidget(quitBtn);

  mainLayout->addLayout(row5);

  // Resize window to fit contents
  window.adjustSize();
  window.setFixedSize(window.sizeHint());

  // Show the window first - the NSWindow must exist before we can modify it
  window.show();

  // Process events to ensure the native window is fully created
  app.processEvents();

  // Apply non-activating behavior (must be done after window is shown)
  Ui::makeNonActivating(&window);

  qDebug() << "[main] Keyboard window shown, entering event loop";

  return app.exec();
}
