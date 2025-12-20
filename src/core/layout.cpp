#include "layout.hpp"
#include <algorithm>
#include <map>
namespace layout {

Element ElementBuilder::addKey(backend::Key key, int row, int column,
                               float widthAsUnit, float heightAsUnit,
                               bool toggle, int holdThresholdMs) {
  auto *btn = new ui::Widget::RightClickableToolButton(parent_);
  auto input = std::make_unique<core::Input>(key, btn, backend_);
  if (toggle) {
    input->setToggleMode(true);
  }

  // Configure per-key hold threshold: short presses (shorter than this)
  // are treated as taps; holding beyond the threshold sends a single keyDown
  // when the threshold elapses and a keyUp on release.
  input->setHoldThresholdMs(holdThresholdMs);

  return Element(
      std::move(input),
      Element::Size{.widthAsUnit = widthAsUnit, .heightAsUnit = heightAsUnit},
      Element::Position{.row = row, .column = column});
}

QVBoxLayout *toQtLayout(const std::vector<Element> &elements) {
  auto mainLayout = std::make_unique<QVBoxLayout>();
  mainLayout->setContentsMargins(4, 4, 4, 4);
  mainLayout->setSpacing(4);

  // Group element pointers by row index to avoid moving/copying Elements
  // which might own unique_ptrs.
  std::map<int, std::vector<const Element *>> rows;
  for (const auto &element : elements) {
    rows[element.row()].push_back(&element);
  }

  // Iterate through sorted rows
  for (auto &[rowIdx, rowElements] : rows) {
    auto rowLayout = std::make_unique<QHBoxLayout>();
    rowLayout->setSpacing(4);

    // Sort element pointers within the row by their column index
    std::ranges::sort(rowElements, [](const Element *lhs, const Element *rhs) {
      return lhs->column() < rhs->column();
    });

    float maxRowHeightUnit = 0.0F;
    for (const auto *element : rowElements) {
      maxRowHeightUnit = std::max(maxRowHeightUnit, element->heightAsUnit());
    }
    if (maxRowHeightUnit <= 0.0F) {
      maxRowHeightUnit = 1.0F;
    }

    for (const auto *element : rowElements) {
      auto *btn = element->input()->button();
      if (btn != nullptr) {
        // Use stretch factors for proportional resizing.
        // We multiply by 100 to handle fractional units (e.g., 1.25, 1.5) as
        // integers.
        int hStretch = static_cast<int>(element->widthAsUnit() * 100);

        // Set policy to Expanding in both directions
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // Set a minimum size based on units to prevent keys from collapsing
        // entirely
        constexpr int baseUnit = 40;
        btn->setMinimumSize(
            static_cast<int>(element->widthAsUnit() * baseUnit),
            static_cast<int>(element->heightAsUnit() * baseUnit));

        rowLayout->addWidget(btn, hStretch);
      }
    }

    // Apply vertical stretch to the row layout
    int vStretch = static_cast<int>(maxRowHeightUnit * 100);
    mainLayout->addLayout(rowLayout.release(), vStretch);
  }

  return mainLayout.release();
}

} // namespace layout
