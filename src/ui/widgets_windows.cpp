#ifdef _WIN32
#include "ui/widgets.hpp"
#include <windows.h>

namespace ui {

void initializeAppleApp() {
  // No-op on Windows
}

void makeNonActivating(QWidget *window) {
  if (!window)
    return;

  // On Windows, we use the WS_EX_NOACTIVATE extended window style.
  // This prevents the window from becoming the active window when the user
  // clicks on it.
  HWND hwnd = reinterpret_cast<HWND>(window->winId());
  if (hwnd) {
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE,
                     exStyle | WS_EX_NOACTIVATE | WS_EX_TOPMOST);
  }

  // Also set Qt attributes as a fallback/complement
  window->setAttribute(Qt::WA_ShowWithoutActivating);
}

} // namespace ui
#endif
