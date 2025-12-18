// ui.hpp
#pragma once

#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QToolButton>
#include <QWidget>

namespace Ui {

namespace Widget {

// Event filter that blocks activation and focus events at the application level
class NoActivationEventFilter : public QObject {
public:
  explicit NoActivationEventFilter(QObject *parent = nullptr)
      : QObject(parent) {}

protected:
  bool eventFilter(QObject *obj, QEvent *event) override {
    switch (event->type()) {
    case QEvent::ApplicationActivate:
    case QEvent::WindowActivate:
    case QEvent::FocusIn:
    case QEvent::FocusAboutToChange:
      // Block these events to prevent activation
      qDebug() << "[NoActivationEventFilter] Blocking event:" << event->type();
      return true; // Event handled (blocked)
    default:
      break;
    }
    return QObject::eventFilter(obj, event);
  }
};

// Custom button that handles right-clicks by triggering the default action
class RightClickableToolButton : public QToolButton {
public:
  explicit RightClickableToolButton(QWidget *parent = nullptr)
      : QToolButton(parent) {
    setContextMenuPolicy(Qt::NoContextMenu); // Disable default context menu
    setFocusPolicy(Qt::NoFocus);             // Disable focus
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    qDebug() << "[RightClickableToolButton] Button created";
  }

  explicit RightClickableToolButton(QLayout *layout, QWidget *parent = nullptr)
      : QToolButton(parent) {
    // Layout parameter is currently unused to avoid incomplete type issues.
    Q_UNUSED(layout);
    setContextMenuPolicy(Qt::NoContextMenu); // Disable default context menu
    setFocusPolicy(Qt::NoFocus);             // Disable focus
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    qDebug() << "[RightClickableToolButton] Button created (layout ctor)";
  }

protected:
  void mousePressEvent(QMouseEvent *event) override {
    if (event->button() == Qt::RightButton) {
      // Create a new event with left button instead of right button
      QMouseEvent leftClickEvent(event->type(), event->position(),
                                 event->globalPosition(), Qt::LeftButton,
                                 Qt::LeftButton, event->modifiers());
      QToolButton::mousePressEvent(&leftClickEvent);
      event->accept();
      return;
    }

    // Default handling for left-clicks and other buttons
    QToolButton::mousePressEvent(event);
  }

  void mouseReleaseEvent(QMouseEvent *event) override {
    if (event->button() == Qt::RightButton) {
      // Create a new event with left button instead of right button
      QMouseEvent leftClickEvent(event->type(), event->position(),
                                 event->globalPosition(), Qt::LeftButton,
                                 Qt::LeftButton, event->modifiers());
      QToolButton::mouseReleaseEvent(&leftClickEvent);
      event->accept();
      return;
    }

    // Default handling for left-clicks and other buttons
    QToolButton::mouseReleaseEvent(event);
  }

  // Prevent focus
  void focusInEvent(QFocusEvent *event) override {
    event->ignore();
    clearFocus();
  }
};
} // namespace Widget

// Make a window non-activating (overlay behavior on macOS)
// This prevents the window from stealing focus from other applications
void makeNonActivating(QWidget *window);

// Install the global event filter on the application
inline void installNoActivationFilter(QApplication *app) {
  static Widget::NoActivationEventFilter filter(app);
  static bool installed = false;
  if (!installed) {
    app->installEventFilter(&filter);
    qDebug() << "[Ui] Installed NoActivationEventFilter";
    installed = true;
  }
}

} // namespace Ui
