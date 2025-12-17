#include <QAction> // Encapsulates an action that can be triggered by UI elements
#include <QApplication> // Core application class for Qt GUI programs
#include <QDebug>       // Provides debug output functionality
#include <QLabel>       // Simple widget to display text or images
#include <QMouseEvent> // Provides mouse event information (e.g., button clicks)
#include <QToolButton> // Button widget that can display an icon and text
#include <QVBoxLayout> // Vertical box layout for arranging widgets
#include <QWidget>     // Base class for all UI objects

// Custom button that handles right-clicks by triggering the default action
class RightClickableToolButton : public QToolButton {
public:
  explicit RightClickableToolButton(QWidget *parent = nullptr)
      : QToolButton(parent) {
    setContextMenuPolicy(Qt::NoContextMenu); // Disable default context menu
    qDebug() << "[RightClickableToolButton] Button created";
  }

protected:
  void mousePressEvent(QMouseEvent *e) override {

    if (e->button() == Qt::RightButton) {
      // Create a new event with left button instead of right button
      QMouseEvent leftClickEvent(e->type(), e->position(), e->globalPosition(),
                                 Qt::LeftButton, Qt::LeftButton,
                                 e->modifiers());
      QToolButton::mousePressEvent(&leftClickEvent);
      e->accept();
      return;
    }

    // Default handling for left-clicks and other buttons
    QToolButton::mousePressEvent(e);
  }

  void mouseReleaseEvent(QMouseEvent *e) override {

    if (e->button() == Qt::RightButton) {
      // Create a new event with left button instead of right button
      QMouseEvent leftClickEvent(e->type(), e->position(), e->globalPosition(),
                                 Qt::LeftButton, Qt::LeftButton,
                                 e->modifiers());
      QToolButton::mouseReleaseEvent(&leftClickEvent);
      e->accept();
      return;
    }

    // Default handling for left-clicks and other buttons
    QToolButton::mouseReleaseEvent(e);
  }
};

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  qDebug() << "[main] Application started";

  // Create main window widget
  QWidget window;
  window.setWindowTitle("Qt + Meson + Conan Example");
  window.resize(360, 150);
  // Lock the window to prevent resizing
  window.setFixedSize(window.size());

  // Create action for the button
  auto *action = new QAction("Run", &window);
  qDebug() << "[main] Action created:" << action->text();

  // Create the custom button and add to layout
  auto *btn = new RightClickableToolButton(&window);
  btn->setDefaultAction(action); // Bind the button to the action
  btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  qDebug() << "[main] Button added to layout";

  // Show the window
  window.show();
  qDebug() << "[main] Window shown, entering event loop";

  return app.exec();
}
