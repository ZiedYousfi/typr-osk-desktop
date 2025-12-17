// ui.hpp
#pragma once

#include <QDebug>
#include <QMouseEvent>
#include <QToolButton>
#include <QWidget>

namespace Ui {

// Custom button that handles right-clicks by triggering the default action
class RightClickableToolButton : public QToolButton {
public:
  explicit RightClickableToolButton(QWidget *parent = nullptr)
      : QToolButton(parent) {
    setContextMenuPolicy(Qt::NoContextMenu); // Disable default context menu
    setFocusPolicy(Qt::NoFocus);             // Disable focus
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

// Make a window non-activating (overlay behavior on macOS)
// This prevents the window from stealing focus from other applications
void makeNonActivating(QWidget *w);

} // namespace Ui