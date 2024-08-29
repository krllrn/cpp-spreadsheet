#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <stack>

// Реализуйте следующие методы
Cell::Cell(SheetInterface* sheet)
: impl_(std::make_unique<EmptyImpl>()),
sheet_(sheet)
{
}

Cell::~Cell() {
    Clear();
}

void Cell::Clear() {
    Set("");
}

void Cell::Set(std::string text) {
    switch (Cell::GetCellType(text))
    {
    case CellType::EMPTY:
        impl_ = std::make_unique<EmptyImpl>();
        break;
    case CellType::TEXT:
        impl_ = std::make_unique<TextImpl>(text);
        break;
    case CellType::FORMULA:
    {
        std::string tmp = this->GetText();
        if (!HasCircularDependency(text)) {
            impl_ = std::make_unique<FormulaImpl>(sheet_, text);
        } else {
            this->Set(tmp);
            throw CircularDependencyException("");
        }
        break;
    }        
    default:
        break;
    }
}

Cell::Value Cell::GetValue() const {
    if (cache_.has_value()) {
        if (std::holds_alternative<double>(cache_.value())) {
            return std::get<double>(cache_.value());
        } else {
            return std::get<FormulaError>(cache_.value());
        }
    }

    Cell::Value value = impl_->GetValue();

    if (impl_->GetType() == CellType::FORMULA) {
        if (std::holds_alternative<double>(value)) {
            cache_ = std::get<double>(value);
        } else {
            cache_ = std::get<FormulaError>(value);
        }
    }
    
    return value;
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

Cell::CellType Cell::GetCellType() const
{
    return impl_->GetType();
}

void Cell::ResetCache()
{
    cache_.reset();
    for (const auto& cell : link_from_) {
        cell->ResetCache();
    }
}

void Cell::AddLinkFrom(Cell* cell)
{
    this->is_referenced_ = true;
    link_from_.insert(cell);
}

bool Cell::HasCircularDependency(std::string text)
{
    std::stack<CellInterface*> stack;
    std::unordered_set<CellInterface*> visited;
    this->impl_ = std::make_unique<FormulaImpl>(sheet_, text);
    
    stack.push(this);

    while (!stack.empty())
    {
        CellInterface* cell = stack.top();
        stack.pop();
        if (cell != nullptr) {
            if (visited.count(cell) == 0) {
                visited.insert(cell);
                for (const auto& link_cell : cell->GetReferencedCells()) {
                    stack.push(sheet_->GetCell(link_cell));
                }
            } else {
                return true;
            }
        }
    }
    
    return false;
}

std::vector<Position> Cell::GetReferencedCells() const
{
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const
{
    return is_referenced_;
}

Cell::EmptyImpl::EmptyImpl()
{
}

Cell::Value Cell::EmptyImpl::GetValue() const
{
    return .0;
}

std::string Cell::EmptyImpl::GetText() const
{
    return text_;
}

Cell::CellType Cell::EmptyImpl::GetType() const
{
    return CellType::EMPTY;
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const
{
    return std::vector<Position>{};
}

Cell::TextImpl::TextImpl(std::string text)
: text_(text)
{
}

Cell::Value Cell::TextImpl::GetValue() const
{
    return text_[0] == '\'' ? text_.substr(1, text_.size()) : text_;
}

std::string Cell::TextImpl::GetText() const
{
    return text_;
}

Cell::CellType Cell::TextImpl::GetType() const
{
    return CellType::TEXT;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const
{
    return std::vector<Position>{};
}

Cell::FormulaImpl::FormulaImpl(SheetInterface* sheet, std::string text) 
: formula_(ParseFormula(text.substr(1, text.size()))),
sheet_(sheet)
{
    text_ = "=" + formula_->GetExpression();
}

Cell::Value Cell::FormulaImpl::GetValue() const
{
    FormulaInterface::Value value = formula_->Evaluate(*sheet_);
    if (std::holds_alternative<double>(value)) {
        return std::get<double>(value);
    }
    return std::get<FormulaError>(value);
}

std::string Cell::FormulaImpl::GetText() const
{
    return text_;
}

Cell::CellType Cell::FormulaImpl::GetType() const
{
    return CellType::FORMULA;
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const
{
    return formula_->GetReferencedCells();
}

Cell::CellType Cell::GetCellType(std::string_view text) {
    if (text[0] == '=' && text.size() > 1) {
        return Cell::CellType::FORMULA;
    } else if (!text.empty()) {
        return Cell::CellType::TEXT;
    } else {
        return Cell::CellType::EMPTY;
    }
}