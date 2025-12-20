#include "ui/window.hpp"
#include "ui/ui.hpp"

namespace Ui {

Window::Window(QWidget *parent) : QWidget(parent) {}

Window::~Window() = default;

void Window::initialize(WindowFlag flags, QVBoxLayout *layout,
                        std::string_view title) {
  if (layout != nullptr) {
    setLayout(layout);
  }

  // Convert the std::string_view to a QString before passing it to
  // setWindowTitle.
  setWindowTitle(
      QString::fromUtf8(title.data(), static_cast<int>(title.size())));

  applyAttributes(flags);
  setWindowFlags(resolveFlags(flags));

  // Ensure the window has a native handle.
  winId();
  window_ = windowHandle();

  if (!hasWindowFlag(flags, WindowFlag::AcceptsFocus)) {
    makeNonActivating(this);
  }
}

void Window::applyAttributes(WindowFlag flags) {
  setAttribute(Qt::WA_ShowWithoutActivating, true);
  setAttribute(Qt::WA_TranslucentBackground,
               hasWindowFlag(flags, WindowFlag::Transparent));

  if (!hasWindowFlag(flags, WindowFlag::AcceptsFocus)) {
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_NoMousePropagation, true);
  }
}

// Resolve the custom ``WindowFlag`` bitmask to the corresponding Qt window
// flags. The function returns a ``Qt::WindowFlags`` value that can be passed
// directly to
// ``setWindowFlags``. The mapping follows the documentation in ``window.hpp``:
//
// * ``None`` – base flags are ``Qt::Tool`` plus the
// ``Qt::WA_ShowWithoutActivating``
//   attribute (already set in ``applyAttributes``).
// * ``StaysOnTop`` – adds ``Qt::WindowStaysOnTopHint``.
// * ``AcceptsFocus`` – when *not* set we add ``Qt::WindowDoesNotAcceptFocus``
// to
//   explicitly prevent the window from receiving focus.
// * ``Transparent`` – handled via the ``Qt::WA_TranslucentBackground``
// attribute;
//   no extra ``Qt::WindowFlags`` are required.
// * ``Decorated`` – when set we replace the ``Qt::Tool`` type with
// ``Qt::Window``
//   to obtain a native frame.
//
// The implementation uses ``hasWindowFlag`` to test individual bits.
[[nodiscard]] Qt::WindowFlags Window::resolveFlags(WindowFlag flags) {
  // Start with a tool window (matches the ``None`` description).
  Qt::WindowFlags result = Qt::Tool;

  // Stays‑on‑top hint.
  if (hasWindowFlag(flags, WindowFlag::StaysOnTop)) {
    result |= Qt::WindowStaysOnTopHint;
  }

  // Focus handling – if the ``AcceptsFocus`` flag is *not* present, request
  // that the window does not accept focus.
  if (!hasWindowFlag(flags, WindowFlag::AcceptsFocus)) {
    result |= Qt::WindowDoesNotAcceptFocus;
  }

  // Decoration – when the caller explicitly asks for a decorated window we
  // replace the ``Qt::Tool`` flag with ``Qt::Window`` which provides the native
  // frame.
  if (hasWindowFlag(flags, WindowFlag::Decorated)) {
    result &= ~Qt::Tool;
    result |= Qt::Window;
  }
  
  // Frameless – when the caller explicitly asks for a frameless window we
  // replace the ``Qt::Window`` flag with ``Qt::FramelessWindowHint`` which
  // removes the native frame.
  if (hasWindowFlag(flags, WindowFlag::Frameless)) {
    result &= ~Qt::Window;
    result |= Qt::FramelessWindowHint;
  }

  return result;
}

} // namespace Ui
