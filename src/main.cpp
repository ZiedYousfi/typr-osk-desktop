#include <QAction> // Encapsulates an action that can be triggered by UI elements
#include <QApplication> // Core application class for Qt GUI programs
#include <QDebug>       // Provides debug output functionality
#include <QHBoxLayout>  // Horizontal box layout for arranging widgets
#include <QWidget>      // Base class for all UI objects

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

  window.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                        Qt::WindowDoesNotAcceptFocus | Qt::Tool);

  window.setAttribute(Qt::WA_ShowWithoutActivating);
  window.setAttribute(Qt::WA_TranslucentBackground);

  // Create horizontal layout
  QHBoxLayout layout(&window);
  layout.setContentsMargins(4, 4, 4, 4);
  layout.setSpacing(4);

  // Create the run button using the new Core::Input class
  // Qt takes ownership of widgets when added to layout with a parent
  Ui::Widget::RightClickableToolButton runBtn(&window);
  Core::Input runInput(input::Key::I, &runBtn, &keyboard);
  layout.addWidget(&runBtn);
  qDebug() << "[main] Run button added to layout";

  // Create action for the quit button (parent takes ownership)
  QAction quitAction("Quit", &window);
  qDebug() << "[main] Quit action created:" << quitAction.text();

  QObject::connect(&quitAction, &QAction::triggered, [&]() {
    qDebug() << "[main] Quit action triggered";
    app.quit();
  });

  // Create the quit button and add to layout
  Ui::Widget::RightClickableToolButton quitBtn(&window);
  quitBtn.setDefaultAction(&quitAction);
  quitBtn.setToolButtonStyle(Qt::ToolButtonTextOnly);
  layout.addWidget(&quitBtn);
  qDebug() << "[main] Quit button added to layout";

  // Resize window to fit contents
  window.adjustSize();
  window.setFixedSize(window.sizeHint());

  // Show the window first - the NSWindow must exist before we can modify it
  window.show();

  // Process events to ensure the native window is fully created
  app.processEvents();

  // Apply non-activating behavior (must be done after window is shown)
  Ui::makeNonActivating(&window);

  qDebug() << "[main] Window shown, entering event loop";

  return app.exec();
}
