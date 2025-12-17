#include <QAction> // Encapsulates an action that can be triggered by UI elements
#include <QApplication> // Core application class for Qt GUI programs
#include <QDebug>       // Provides debug output functionality
#include <QHBoxLayout>  // Horizontal box layout for arranging widgets
#include <QWidget>      // Base class for all UI objects

#include "input/input.hpp"
#include "ui/ui.hpp"

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  qDebug() << "[main] Application started";

  // Install event filter to block activation events at the Qt level
  Ui::installNoActivationFilter(&app);

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
  auto *layout = new QHBoxLayout(&window);
  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(4);

  // Create action for the I button
  auto *runAction = new QAction("I", &window);
  qDebug() << "[main] Run action created:" << runAction->text();

  QObject::connect(runAction, &QAction::triggered, [&]() {
    qDebug() << "[main] Run action triggered";
    keyboard.tap(input::Key::I);
  });

  // Create the I button and add to layout
  auto *runBtn = new Ui::RightClickableToolButton(&window);
  runBtn->setDefaultAction(runAction);
  runBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
  layout->addWidget(runBtn);
  qDebug() << "[main] Run button added to layout";

  // Create action for the quit button
  auto *quitAction = new QAction("Quit", &window);
  qDebug() << "[main] Quit action created:" << quitAction->text();

  QObject::connect(quitAction, &QAction::triggered, [&]() {
    qDebug() << "[main] Quit action triggered";
    app.quit();
  });

  // Create the quit button and add to layout
  auto *quitBtn = new Ui::RightClickableToolButton(&window);
  quitBtn->setDefaultAction(quitAction);
  quitBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
  layout->addWidget(quitBtn);
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
