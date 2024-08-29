#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <stack>

using namespace std::literals;

Sheet::~Sheet()
{
    sheet_.clear();
    row_none_empty_cells_.clear();
    col_none_empty_cells_.clear();
    print_size_ = {0, 0};
}

inline std::ostream& operator<<(std::ostream& output, Position pos) {
    return output << "(" << pos.row << ", " << pos.col << ")";
}

void CheckPositionIsValid(const Position& pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position.");
    }
}

bool Sheet::IsValidCell(const Position& pos) const {
    if (sheet_.count(pos) == 0 || 
        sheet_.at(pos) == nullptr || 
        !(pos < Position{print_size_.rows, print_size_.cols})) 
    {
        return false;
    }

    return true;
}

void Sheet::FillLinks(std::unique_ptr<Cell>& main_cell) {
    auto ref_cells = main_cell->GetReferencedCells();
    if (ref_cells.size() > 0) {
        for (const auto& pos_ref : main_cell->GetReferencedCells()) {
            if (sheet_.count(pos_ref) == 0) {
                SetCell(pos_ref, "");
            } 
            sheet_.at(pos_ref)->AddLinkFrom(main_cell.get());
        }
    }
}

void Sheet::SetCell(Position pos, std::string text) {
    CheckPositionIsValid(pos);

    if (sheet_.count(pos) == 0) {
        sheet_[pos] = std::make_unique<Cell>(this);
    } else if (sheet_.at(pos)->IsReferenced()) {
        sheet_.at(pos)->ResetCache();
    }
    
    try {
        sheet_.at(pos)->Set(text);
    } catch (const CircularDependencyException&) {
        throw;
    } catch (const std::exception& exc) {
        throw FormulaException(exc.what());
    }

    FillLinks(sheet_.at(pos));

    if (!(sheet_.at(pos) == nullptr || sheet_.at(pos)->GetText().empty())) {
        IncreasePrintableSize(pos);
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    CheckPositionIsValid(pos);

    if (!IsValidCell(pos)) {
        return nullptr;
    }
    return sheet_.at(pos).get();
}

CellInterface* Sheet::GetCell(Position pos) {
    CheckPositionIsValid(pos);

    if (!IsValidCell(pos)) {
        return nullptr;
    }
    return sheet_.at(pos).get();
}

void Sheet::ClearCell(Position pos) {
    CheckPositionIsValid(pos);

    if (pos < Position{print_size_.rows, print_size_.cols} && sheet_.count(pos) != 0) {
        sheet_.at(pos)->Clear();
        sheet_.at(pos) = nullptr;
        DecreasePrintableSize(pos);
    }
}

Size Sheet::GetPrintableSize() const {
    return print_size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int r = 0; r < print_size_.rows; ++r) {
        for (int c = 0; c < print_size_.cols; ++c) {
            if (GetCell(Position{r, c}) != nullptr) {
                std::visit([&output](const auto& arg){output << arg;}, GetCell(Position{r, c})->GetValue());;
            }
            if (c < print_size_.cols - 1) {
                output << "\t";
            }            
        }
        output << "\n";
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int r = 0; r < print_size_.rows; ++r) {
        for (int c = 0; c < print_size_.cols; ++c) {
            Position pos = Position{r, c};
            if (GetCell(pos) != nullptr) {
                output << sheet_.at(pos)->GetText();
            }
            if (c < print_size_.cols - 1) {
                output << "\t";
            }            
        }
        output << "\n";
    }
}



void Sheet::IncreasePrintableSize(const Position& pos)
{
    ++row_none_empty_cells_[pos.row];
    ++col_none_empty_cells_[pos.col];
    print_size_.rows = pos.row >= print_size_.rows ? pos.row + 1 : print_size_.rows;
    print_size_.cols = pos.col >= print_size_.cols ? pos.col + 1 : print_size_.cols;
}

void Sheet::DecreasePrintableSize(const Position& pos)
{
    --row_none_empty_cells_.at(pos.row);
    --col_none_empty_cells_.at(pos.col);
    if (row_none_empty_cells_.at(pos.row) == 0) {
        row_none_empty_cells_.erase(pos.row);
    }
    if (col_none_empty_cells_.at(pos.col) == 0) {
        col_none_empty_cells_.erase(pos.col);
    }
    print_size_.rows = row_none_empty_cells_.empty() ? 0 : std::prev((std::end(row_none_empty_cells_)))->first + 1;
    print_size_.cols = col_none_empty_cells_.empty() ? 0 : std::prev((std::end(col_none_empty_cells_)))->first + 1;
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}