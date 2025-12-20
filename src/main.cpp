#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <unordered_map>
#include <vector>

#include "backend/backend.hpp"
#include "core/layout.hpp"
#include "ui/widgets.hpp"
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

  ui::initializeAppleApp();
  ui::installNoActivationFilter(&app);

  backend::InputBackend keyboard;
  if (!keyboard.isReady()) {
    keyboard.requestPermissions();
  }

  AppState state;

  // --- Main Keyboard Window ---
  ui::Window keyboardWindow;
  state.windows["keyboard"] = &keyboardWindow;

  layout::ElementListBuilder listBuilder(&keyboard, &keyboardWindow);

  // --- Row 0: Numbers & Backspace ---
  listBuilder.addKey(backend::Key::Escape);
  listBuilder.addKey(backend::Key::Grave);
  listBuilder.addKey(backend::Key::Num1);
  listBuilder.addKey(backend::Key::Num2);
  listBuilder.addKey(backend::Key::Num3);
  listBuilder.addKey(backend::Key::Num4);
  listBuilder.addKey(backend::Key::Num5);
  listBuilder.addKey(backend::Key::Num6);
  listBuilder.addKey(backend::Key::Num7);
  listBuilder.addKey(backend::Key::Num8);
  listBuilder.addKey(backend::Key::Num9);
  listBuilder.addKey(backend::Key::Num0);
  listBuilder.addKey(backend::Key::Minus);
  listBuilder.addKey(backend::Key::Equal);
  listBuilder.addKey(backend::Key::Backspace, kUnit2_0);

  // --- Row 1: Tab & QWERTY ---
  listBuilder.nextRow();
  listBuilder.addKey(backend::Key::Tab, kUnit1_5);
  listBuilder.addKey(backend::Key::Q);
  listBuilder.addKey(backend::Key::W);
  listBuilder.addKey(backend::Key::E);
  listBuilder.addKey(backend::Key::R);
  listBuilder.addKey(backend::Key::T);
  listBuilder.addKey(backend::Key::Y);
  listBuilder.addKey(backend::Key::U);
  listBuilder.addKey(backend::Key::I);
  listBuilder.addKey(backend::Key::O);
  listBuilder.addKey(backend::Key::P);
  listBuilder.addKey(backend::Key::LeftBracket);
  listBuilder.addKey(backend::Key::RightBracket);
  listBuilder.addKey(backend::Key::Backslash, kUnit1_5);

  // --- Row 2: Caps & ASDF ---
  listBuilder.nextRow();
  listBuilder.addKey(backend::Key::CapsLock, kUnit1_75, true);
  listBuilder.addKey(backend::Key::A);
  listBuilder.addKey(backend::Key::S);
  listBuilder.addKey(backend::Key::D);
  listBuilder.addKey(backend::Key::F);
  listBuilder.addKey(backend::Key::G);
  listBuilder.addKey(backend::Key::H);
  listBuilder.addKey(backend::Key::J);
  listBuilder.addKey(backend::Key::K);
  listBuilder.addKey(backend::Key::L);
  listBuilder.addKey(backend::Key::Semicolon);
  listBuilder.addKey(backend::Key::Apostrophe);
  listBuilder.addKey(backend::Key::Enter, kUnit2_25);

  // --- Row 3: Shift & ZXCV ---
  listBuilder.nextRow();
  listBuilder.addKey(backend::Key::ShiftLeft, kUnit2_5, true);
  listBuilder.addKey(backend::Key::Z);
  listBuilder.addKey(backend::Key::X);
  listBuilder.addKey(backend::Key::C);
  listBuilder.addKey(backend::Key::V);
  listBuilder.addKey(backend::Key::B);
  listBuilder.addKey(backend::Key::N);
  listBuilder.addKey(backend::Key::M);
  listBuilder.addKey(backend::Key::Comma);
  listBuilder.addKey(backend::Key::Period);
  listBuilder.addKey(backend::Key::Slash);
  listBuilder.addKey(backend::Key::ShiftRight, kUnit2_5, true);

  // --- Row 4: Modifiers & Space ---
  listBuilder.nextRow();
  listBuilder.addKey(backend::Key::CtrlLeft, kUnit1_5, true);
  listBuilder.addKey(backend::Key::AltLeft, kUnit1_5, true);
  listBuilder.addKey(backend::Key::SuperLeft, kUnit1_5, true);
  listBuilder.addKey(backend::Key::Space, kUnit6_25);
  listBuilder.addKey(backend::Key::SuperRight, kUnit1_5, true);
  listBuilder.addKey(backend::Key::AltRight, kUnit1_5, true);

  listBuilder.addKey(backend::Key::Left);
  listBuilder.addKey(backend::Key::Up);
  listBuilder.addKey(backend::Key::Down);
  listBuilder.addKey(backend::Key::Right);

  std::vector<layout::Element> elements = std::move(listBuilder).build();
  auto *mainLayout = layout::toQtLayout(elements);

  keyboardWindow.initialize(ui::Window::WindowFlag::StaysOnTop |
                                ui::Window::WindowFlag::Transparent,
                            mainLayout, "Typr OSK");

  keyboardWindow.adjustSize();
  qDebug() << "[main] Showing keyboard window";
  keyboardWindow.show();

  // --- Toggle Button Window ---
  ui::Window toggleWindow;
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
        ui::makeNonActivating(windowIter->second);
      }
    }
  });

  toggleWindow.initialize(ui::Window::WindowFlag::StaysOnTop |
                              ui::Window::WindowFlag::Transparent |
                              ui::Window::WindowFlag::Frameless,
                          toggleLayout, "Typr Toggle");

  toggleWindow.adjustSize();
  toggleWindow.setFixedSize(toggleWindow.sizeHint());
  qDebug() << "[main] Showing toggle window";
  toggleWindow.show();

  qDebug() << "[main] Processing events";
  app.processEvents();

  qDebug() << "[main] Making keyboard non-activating";
  ui::makeNonActivating(&keyboardWindow);
  ui::makeNonActivating(&toggleWindow);

  qDebug() << "[main] Entering event loop";
  return app.exec();
}
