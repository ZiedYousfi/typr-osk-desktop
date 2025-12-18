#pragma once

#include <memory>
#include <vector>

#include <QVBoxLayout>

#include "input.hpp"

namespace Layout {

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
   * @brief Construct an element with an existing Core::Input.
   * Element takes ownership of the Core::Input.
   */
  Element(std::unique_ptr<Core::Input> input, Size size, Position pos)
      : input_(std::move(input)), size_(size), pos_(pos) {}

  // Move-only semantics to safely manage the unique_ptr<Core::Input>
  Element(Element &&) noexcept = default;
  Element &operator=(Element &&) noexcept = default;
  Element(const Element &) = delete;
  Element &operator=(const Element &) = delete;

  ~Element() = default;

  // Accessors
  [[nodiscard]] Core::Input *input() const { return input_.get(); }
  [[nodiscard]] float widthAsUnit() const { return size_.widthAsUnit; }
  [[nodiscard]] float heightAsUnit() const { return size_.heightAsUnit; }
  [[nodiscard]] int row() const { return pos_.row; }
  [[nodiscard]] int column() const { return pos_.column; }

private:
  std::unique_ptr<Core::Input> input_;
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
  explicit ElementBuilder(input::InputBackend *backend,
                          QWidget *parent = nullptr)
      : backend_(backend), parent_(parent) {}

  /**
   * @brief Creates a Layout::Element.
   * @param key The physical key to map.
   * @param row The logical row in the grid.
   * @param column The logical column in the grid.
   * @param widthAsUnit The relative width of the key.
   * @param toggle Whether the key is in toggle mode.
   * @return A constructed Layout::Element.
   */
  [[nodiscard]] Element addKey(input::Key key, int row, int column,
                               float widthAsUnit = 1.0F, bool toggle = false);

private:
  input::InputBackend *backend_;
  QWidget *parent_;
};

/**
 * @brief Abstraction to build a list of elements row by row.
 */
class ElementListBuilder {
public:
  explicit ElementListBuilder(input::InputBackend *backend,
                              QWidget *parent = nullptr)
      : builder_(backend, parent) {}

  /**
   * @brief Adds a key to the current row and advances the column.
   */
  void addKey(input::Key key, float widthAsUnit = 1.0F, bool toggle = false) {
    elements_.push_back(
        builder_.addKey(key, currentRow_, currentCol_++, widthAsUnit, toggle));
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

} // namespace Layout
