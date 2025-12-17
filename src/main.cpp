#include <QAction> // Encapsulates an action that can be triggered by UI elements
#include <QApplication> // Core application class for Qt GUI programs
#include <QDebug>       // Provides debug output functionality
#include <QLabel>       // Simple widget to display text or images
#include <QVBoxLayout>  // Vertical box layout for arranging widgets
#include <QWidget>      // Base class for all UI objects

#include "input/input.hpp"
#include "ui/ui.hpp"

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  qDebug() << "[main] Application started";

  input::InputBackend keyboard;

  if (!keyboard.isReady()) {
    keyboard.requestPermissions(); // Shows macOS dialog
  }

  // Create main window widget
  QWidget window;
  window.setWindowTitle("Qt + Meson + Conan Example");
  window.resize(360, 150);
  // Lock the window to prevent resizing
  window.setFixedSize(window.size());

  // Make the window only an overlay and not take focus
  window.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                        Qt::WindowDoesNotAcceptFocus);
  
  Ui::makeNonActivating(&window);

  // Create action for the button
  auto *action = new QAction("Run", &window);
  qDebug() << "[main] Action created:" << action->text();

  QObject::connect(action, &QAction::triggered, [&]() {
    qDebug() << "[main] Action triggered";
    keyboard.tap(input::Key::I);
  });

  // Create the custom button and add to layout
  auto *btn = new Ui::RightClickableToolButton(&window);
  btn->setDefaultAction(action); // Bind the button to the action
  btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  qDebug() << "[main] Button added to layout";

  // Show the window
  window.show();
  qDebug() << "[main] Window shown, entering event loop";

  return app.exec();
}
