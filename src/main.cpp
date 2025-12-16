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
    qDebug() << "[RightClickableToolButton] Mouse press event, button:"
             << e->button();

    if (e->button() == Qt::RightButton) { // Detect right mouse button
      qDebug() << "[RightClickableToolButton] Right-click detected";

      if (auto a = defaultAction(); a && isEnabled()) {
        qDebug() << "[RightClickableToolButton] Triggering default action:"
                 << a->text();
        a->trigger();
        e->accept();
        return;
      }

      if (isEnabled()) {
        qDebug() << "[RightClickableToolButton] No default action, performing "
                    "normal click";
        click();
        e->accept();
        return;
      }
    }

    // Default handling for left-clicks and other buttons
    QToolButton::mousePressEvent(e);
  }
};

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  qDebug() << "[main] Application started";

  // Create main window widget
  QWidget window;
  window.setWindowTitle("Qt + Meson + Conan Example");
  window.resize(360, 150);

  // Create a vertical layout for the window
  auto *layout = new QVBoxLayout(&window);

  // Create and add label
  auto *label = new QLabel("Hello Qt + Meson + Conan!");
  label->setAlignment(Qt::AlignCenter);
  layout->addWidget(label);

  // Create action for the button
  auto *action = new QAction("Run", &window);
  action->setCheckable(true); // Enable toggle state (checked/unchecked)
  qDebug() << "[main] Action created:" << action->text();

  // Connect action to log when triggered
  QObject::connect(action, &QAction::triggered, &app, [=](bool checked) {
    qDebug() << "[main] Action triggered! Checked state:" << checked;
  });

  // Create the custom button and add to layout
  auto *btn = new RightClickableToolButton(&window);
  btn->setDefaultAction(action); // Bind the button to the action
  btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  layout->addWidget(btn);
  qDebug() << "[main] Button added to layout";

  // Show the window
  window.show();
  qDebug() << "[main] Window shown, entering event loop";

  return app.exec();
}