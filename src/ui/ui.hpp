// ui.hpp
#pragma once

#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QToolButton>
#include <QWidget>

namespace Ui {

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

  // Prevent focus
  void focusInEvent(QFocusEvent *e) override {
    e->ignore();
    clearFocus();
  }
};

// Make a window non-activating (overlay behavior on macOS)
// This prevents the window from stealing focus from other applications
void makeNonActivating(QWidget *w);

// Install the global event filter on the application
inline void installNoActivationFilter(QApplication *app) {
  static NoActivationEventFilter *filter = nullptr;
  if (!filter) {
    filter = new NoActivationEventFilter(app);
    app->installEventFilter(filter);
    qDebug() << "[Ui] Installed NoActivationEventFilter";
  }
}

} // namespace Ui
