#pragma once

#include "common.h"
#include "formula.h"

#include <optional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
    enum CellType {
        EMPTY,
        FORMULA,
        TEXT
    };

    Cell(SheetInterface* sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    CellType GetCellType() const;

    void ResetCache();

    void AddLinkFrom(Cell* cell);

    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

private:
    class Impl {
        public:
            virtual ~Impl() = default;

            virtual Cell::Value GetValue() const = 0;

            virtual std::string GetText() const = 0;

            virtual CellType GetType() const = 0;

            virtual std::vector<Position> GetReferencedCells() const = 0;
    };

    class EmptyImpl : public Impl {
        public:
            EmptyImpl();

            ~EmptyImpl() = default;

            Cell::Value GetValue() const override;

            std::string GetText() const override;

            CellType GetType() const override;

            std::vector<Position> GetReferencedCells() const override;

        private:
            std::string text_ = {};
    };

    class TextImpl : public Impl {
        public:
            TextImpl(std::string text);

            ~TextImpl() = default;

            Cell::Value GetValue() const override;

            std::string GetText() const override;

            CellType GetType() const override;

            std::vector<Position> GetReferencedCells() const override;

        private:
            std::string text_;
    };

    class FormulaImpl : public Impl {
        public:
            FormulaImpl(SheetInterface* sheet, std::string text);

            ~FormulaImpl() = default;

            Cell::Value GetValue() const override;

            std::string GetText() const override;

            CellType GetType() const override;

            std::vector<Position> GetReferencedCells() const override;

        private:
            std::unique_ptr<FormulaInterface> formula_;
            std::string text_;
            SheetInterface* sheet_;
    };
    
    std::unique_ptr<Impl> impl_;
    mutable std::optional<FormulaInterface::Value> cache_ = std::nullopt;
    SheetInterface* sheet_;
    bool is_referenced_ = false;
    std::unordered_set<Cell*> link_from_;

    CellType GetCellType(std::string_view text);
    bool HasCircularDependency(std::string text);
};