#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <map>
#include <unordered_map>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    Size print_size_;
    std::unordered_map<Position, std::unique_ptr<Cell>, Position::Hasher, Position::EqualTo> sheet_;
    std::map<int, int> row_none_empty_cells_;
    std::map<int, int> col_none_empty_cells_;

    void IncreasePrintableSize(const Position& pos);
    void DecreasePrintableSize(const Position& pos);
    void FillLinks(std::unique_ptr<Cell>& main_cell);
};