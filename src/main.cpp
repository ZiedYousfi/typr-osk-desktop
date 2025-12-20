#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <unordered_map>
#include <vector>

#include "core/layout.hpp"
#include "input/input.hpp"
#include "ui/ui.hpp"
#include "ui/window.hpp"

namespace {
// Layout constants
constexpr float kUnit1_5 = 1.5F;   // 60px / 40px
constexpr float kUnit1_75 = 1.75F; // 70px / 40px
constexpr float kUnit2_0 = 2.0F;   // 80px / 40px
constexpr float kUnit2_25 = 2.25F; // 90px / 40px
constexpr float kUnit2_5 = 2.5F;   // 100px / 40px
constexpr float kUnit6_25 = 6.25F; // 250px / 40px

struct AppState {
  std::unordered_map<std::string, QWidget *> windows;
};
} // namespace

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  qDebug() << "[main] Application started";

  Ui::initializeAppleApp();
  Ui::installNoActivationFilter(&app);

  input::InputBackend keyboard;
  if (!keyboard.isReady()) {
    keyboard.requestPermissions();
  }

  AppState state;

  // --- Main Keyboard Window ---
  Ui::Window keyboardWindow;
  state.windows["keyboard"] = &keyboardWindow;

  Layout::ElementListBuilder listBuilder(&keyboard, &keyboardWindow);

  // --- Row 0: Numbers & Backspace ---
  listBuilder.addKey(input::Key::Escape);
  listBuilder.addKey(input::Key::Grave);
  listBuilder.addKey(input::Key::Num1);
  listBuilder.addKey(input::Key::Num2);
  listBuilder.addKey(input::Key::Num3);
  listBuilder.addKey(input::Key::Num4);
  listBuilder.addKey(input::Key::Num5);
  listBuilder.addKey(input::Key::Num6);
  listBuilder.addKey(input::Key::Num7);
  listBuilder.addKey(input::Key::Num8);
  listBuilder.addKey(input::Key::Num9);
  listBuilder.addKey(input::Key::Num0);
  listBuilder.addKey(input::Key::Minus);
  listBuilder.addKey(input::Key::Equal);
  listBuilder.addKey(input::Key::Backspace, kUnit2_0);

  // --- Row 1: Tab & QWERTY ---
  listBuilder.nextRow();
  listBuilder.addKey(input::Key::Tab, kUnit1_5);
  listBuilder.addKey(input::Key::Q);
  listBuilder.addKey(input::Key::W);
  listBuilder.addKey(input::Key::E);
  listBuilder.addKey(input::Key::R);
  listBuilder.addKey(input::Key::T);
  listBuilder.addKey(input::Key::Y);
  listBuilder.addKey(input::Key::U);
  listBuilder.addKey(input::Key::I);
  listBuilder.addKey(input::Key::O);
  listBuilder.addKey(input::Key::P);
  listBuilder.addKey(input::Key::LeftBracket);
  listBuilder.addKey(input::Key::RightBracket);
  listBuilder.addKey(input::Key::Backslash, kUnit1_5);

  // --- Row 2: Caps & ASDF ---
  listBuilder.nextRow();
  listBuilder.addKey(input::Key::CapsLock, kUnit1_75, true);
  listBuilder.addKey(input::Key::A);
  listBuilder.addKey(input::Key::S);
  listBuilder.addKey(input::Key::D);
  listBuilder.addKey(input::Key::F);
  listBuilder.addKey(input::Key::G);
  listBuilder.addKey(input::Key::H);
  listBuilder.addKey(input::Key::J);
  listBuilder.addKey(input::Key::K);
  listBuilder.addKey(input::Key::L);
  listBuilder.addKey(input::Key::Semicolon);
  listBuilder.addKey(input::Key::Apostrophe);
  listBuilder.addKey(input::Key::Enter, kUnit2_25);

  // --- Row 3: Shift & ZXCV ---
  listBuilder.nextRow();
  listBuilder.addKey(input::Key::ShiftLeft, kUnit2_5, true);
  listBuilder.addKey(input::Key::Z);
  listBuilder.addKey(input::Key::X);
  listBuilder.addKey(input::Key::C);
  listBuilder.addKey(input::Key::V);
  listBuilder.addKey(input::Key::B);
  listBuilder.addKey(input::Key::N);
  listBuilder.addKey(input::Key::M);
  listBuilder.addKey(input::Key::Comma);
  listBuilder.addKey(input::Key::Period);
  listBuilder.addKey(input::Key::Slash);
  listBuilder.addKey(input::Key::ShiftRight, kUnit2_5, true);

  // --- Row 4: Modifiers & Space ---
  listBuilder.nextRow();
  listBuilder.addKey(input::Key::CtrlLeft, kUnit1_5, true);
  listBuilder.addKey(input::Key::AltLeft, kUnit1_5, true);
  listBuilder.addKey(input::Key::SuperLeft, kUnit1_5, true);
  listBuilder.addKey(input::Key::Space, kUnit6_25);
  listBuilder.addKey(input::Key::SuperRight, kUnit1_5, true);
  listBuilder.addKey(input::Key::AltRight, kUnit1_5, true);

  listBuilder.addKey(input::Key::Left);
  listBuilder.addKey(input::Key::Up);
  listBuilder.addKey(input::Key::Down);
  listBuilder.addKey(input::Key::Right);

  std::vector<Layout::Element> elements = std::move(listBuilder).build();
  auto *mainLayout = Layout::toQtLayout(elements);

  keyboardWindow.initialize(Ui::Window::WindowFlag::StaysOnTop |
                                Ui::Window::WindowFlag::Transparent,
                            mainLayout, "Typr OSK");

  keyboardWindow.adjustSize();
  qDebug() << "[main] Showing keyboard window";
  keyboardWindow.show();

  // --- Toggle Button Window ---
  Ui::Window toggleWindow;
  state.windows["toggle"] = &toggleWindow;

  auto *toggleLayout = new QVBoxLayout();
  auto *toggleButton = new QPushButton("Toggle Keyboard");
  toggleLayout->addWidget(toggleButton);

  QObject::connect(toggleButton, &QPushButton::clicked, [&state]() {
    auto windowIter = state.windows.find("keyboard");
    if (windowIter != state.windows.end() && windowIter->second != nullptr) {
      if (windowIter->second->isVisible()) {
        windowIter->second->hide();
      } else {
        windowIter->second->show();
        // Re-apply non-activating status if needed after showing
        Ui::makeNonActivating(windowIter->second);
      }
    }
  });

  toggleWindow.initialize(Ui::Window::WindowFlag::StaysOnTop |
                              Ui::Window::WindowFlag::Transparent |
                              Ui::Window::WindowFlag::Frameless,
                          toggleLayout, "Typr Toggle");

  toggleWindow.adjustSize();
  toggleWindow.setFixedSize(toggleWindow.sizeHint());
  qDebug() << "[main] Showing toggle window";
  toggleWindow.show();

  qDebug() << "[main] Processing events";
  app.processEvents();

  qDebug() << "[main] Making keyboard non-activating";
  Ui::makeNonActivating(&keyboardWindow);
  Ui::makeNonActivating(&toggleWindow);

  qDebug() << "[main] Entering event loop";
  return app.exec();
}
