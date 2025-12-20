#pragma once

#include <MacTypes.h>
#include <cstdint>
#include <type_traits>

#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>

namespace Ui {

class Window : public QWidget {
public:
  enum class WindowFlag : uint8_t {
    None = 0,            ///< Qt::Tool + Qt::WA_ShowWithoutActivating
    StaysOnTop = 0x01,   ///< Qt::WindowStaysOnTopHint
    AcceptsFocus = 0x02, ///< no Qt::WindowDoesNotAcceptFocus
    Transparent = 0x04,  ///< Qt::WA_TranslucentBackground
    Decorated = 0x08,    ///< permit native frame hints
    Frameless = 0x10     ///< Qt::FramelessWindowHint
  };

  explicit Window(QWidget *parent = nullptr);
  Window(const Window &) = delete;
  Window(Window &&) = delete;
  Window &operator=(const Window &) = delete;
  Window &operator=(Window &&) = delete;
  ~Window() override;

  void initialize(WindowFlag flags, QVBoxLayout *layout,
                  std::string_view title);

private:
  [[nodiscard]] static Qt::WindowFlags resolveFlags(WindowFlag flags);
  void applyAttributes(WindowFlag flags);

  QWindow *window_ = nullptr;
};

constexpr Window::WindowFlag operator|(Window::WindowFlag lhs,
                                       Window::WindowFlag rhs) noexcept {
  using Underlying = std::underlying_type_t<Window::WindowFlag>;
  return static_cast<Window::WindowFlag>(static_cast<Underlying>(lhs) |
                                         static_cast<Underlying>(rhs));
}

constexpr Window::WindowFlag operator&(Window::WindowFlag lhs,
                                       Window::WindowFlag rhs) noexcept {
  using Underlying = std::underlying_type_t<Window::WindowFlag>;
  return static_cast<Window::WindowFlag>(static_cast<Underlying>(lhs) &
                                         static_cast<Underlying>(rhs));
}

constexpr bool hasWindowFlag(Window::WindowFlag flags,
                             Window::WindowFlag mask) noexcept {
  return (flags & mask) != Window::WindowFlag::None;
}

} // namespace Ui
