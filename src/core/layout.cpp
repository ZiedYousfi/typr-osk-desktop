#include "layout.hpp"
#include <algorithm>
#include <map>

namespace Layout {

Element ElementBuilder::addKey(input::Key key, int row, int column,
                               float widthAsUnit, bool toggle) {
  auto *btn = new Ui::Widget::RightClickableToolButton(parent_);
  auto input = std::make_unique<Core::Input>(key, btn, backend_);
  if (toggle) {
    input->setToggleMode(true);
  }

  return Element(std::move(input), Element::Size{.widthAsUnit=widthAsUnit, .heightAsUnit=1.0F},
                 Element::Position{.row=row, .column=column});
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

    for (const auto *element : rowElements) {
      auto *btn = element->input()->button();
      if (btn != nullptr) {
        // Calculate width: assuming a base unit of 40 pixels
        constexpr int baseUnit = 40;
        int width = static_cast<int>(element->widthAsUnit() * baseUnit);

        btn->setMinimumWidth(width);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        rowLayout->addWidget(btn);
      }
    }

    mainLayout->addLayout(rowLayout.release());
  }

  return mainLayout.release();
}

} // namespace Layout
