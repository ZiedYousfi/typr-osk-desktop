#pragma once

#include <memory>
#include <vector>

#include <QVBoxLayout>

#include "../backend/backend.hpp"
#include "core/input.hpp"

namespace layout {

/**
 * @brief Represents a single UI element (key) in the layout.
 * Holds its logical representation (Core::Input) and its layout properties.
 */
class Element {
public:
  struct Size {
    float widthAsUnit =
        1.0F; ///< Width relative to a standard key unit (e.g., 1.0, 1.5)
    float heightAsUnit = 1.0F; ///< Height relative to a standard key unit
  };

  struct Position {
    int row = 0;    ///< Logical row in the keyboard grid
    int column = 0; ///< Logical column in the keyboard grid
  };

  /**
   * @brief Construct an element with an existing core::Input.
   * Element takes ownership of the core::Input.
   */
  Element(std::unique_ptr<core::Input> input, Size size, Position pos)
      : input_(std::move(input)), size_(size), pos_(pos) {}

  // Move-only semantics to safely manage the unique_ptr<Core::Input>
  Element(Element &&) noexcept = default;
  Element &operator=(Element &&) noexcept = default;
  Element(const Element &) = delete;
  Element &operator=(const Element &) = delete;

  ~Element() = default;

  // Accessors
  [[nodiscard]] core::Input *input() const { return input_.get(); }
  [[nodiscard]] float widthAsUnit() const { return size_.widthAsUnit; }
  [[nodiscard]] float heightAsUnit() const { return size_.heightAsUnit; }
  [[nodiscard]] int row() const { return pos_.row; }
  [[nodiscard]] int column() const { return pos_.column; }

private:
  std::unique_ptr<core::Input> input_;
  Size size_;
  Position pos_;
};

/**
 * @brief Builder for creating Layout::Element objects.
 * Encapsulates the logic of key creation, button instantiation, and
 * positioning.
 */
class ElementBuilder {
public:
  explicit ElementBuilder(backend::InputBackend *backend,
                          QWidget *parent = nullptr)
      : backend_(backend), parent_(parent) {}

  /**
   * @brief Creates a Layout::Element.
   * @param key The physical key to map.
   * @param row The logical row in the grid.
   * @param column The logical column in the grid.
   * @param widthAsUnit The relative width of the key.
   * @param heightAsUnit The relative height of the key.
   * @param toggle Whether the key is in toggle mode.
   * @param holdThresholdMs How many milliseconds the user must hold the
   * button before it is considered held and a keyDown is sent (default: 300).
   * @return A constructed Layout::Element.
   */
  [[nodiscard]] Element addKey(backend::Key key, int row, int column,
                               float widthAsUnit = 1.0F,
                               float heightAsUnit = 1.0F, bool toggle = false,
                               int holdThresholdMs = 300);

private:
  backend::InputBackend *backend_;
  QWidget *parent_;
};

/**
 * @brief Abstraction to build a list of elements row by row.
 */
class ElementListBuilder {
public:
  explicit ElementListBuilder(backend::InputBackend *backend,
                              QWidget *parent = nullptr)
      : builder_(backend, parent) {}

  /**
   * @brief Adds a key to the current row and advances the column.
   *
   * Two overloads are provided for convenience:
   *  - addKey(key, width, height, toggle, holdThresholdMs)
   *  - addKey(key, width, toggle, holdThresholdMs)  // height defaults to 1.0
   *
   * @param holdThresholdMs How many milliseconds the user must hold the
   * button before the key is considered held (default: 300).
   */
  void addKey(backend::Key key, float widthAsUnit = 1.0F,
              float heightAsUnit = 1.0F, bool toggle = false,
              int holdThresholdMs = 300) {
    elements_.push_back(builder_.addKey(key, currentRow_, currentCol_++,
                                        widthAsUnit, heightAsUnit, toggle,
                                        holdThresholdMs));
  }

  // Convenience overload: allow passing 'toggle' as the third parameter
  // (e.g., addKey(key, width, true) to make the key toggle on click).
  void addKey(backend::Key key, float widthAsUnit, bool toggle,
              int holdThresholdMs = 300) {
    elements_.push_back(builder_.addKey(key, currentRow_, currentCol_++,
                                        widthAsUnit, 1.0F, toggle,
                                        holdThresholdMs));
  }

  /**
   * @brief Advances to the next row and resets the column.
   */
  void nextRow() {
    currentRow_++;
    currentCol_ = 0;
  }

  /**
   * @brief Returns the built list of elements.
   * This consumes the builder.
   */
  [[nodiscard]] std::vector<Element> build() && { return std::move(elements_); }

private:
  ElementBuilder builder_;
  std::vector<Element> elements_;
  int currentRow_ = 0;
  int currentCol_ = 0;
};

/**
 * @brief Organizes a collection of Elements into a Qt layout structure.
 * This function groups elements by their row property and creates horizontal
 * rows within a main vertical layout.
 *
 * @param elements The elements to organize.
 * @return A pointer to a new QVBoxLayout. The caller or a parent widget takes
 * ownership of the layout.
 */
QVBoxLayout *toQtLayout(const std::vector<Element> &elements);

} // namespace layout
