#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <set>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
    class Formula : public FormulaInterface {
    public:
        explicit Formula(std::string expression) 
        try
        : ast_(ParseFormulaAST(expression))
        {
        } catch(const std::exception& fex) {
            throw FormulaException(fex.what());
        } 

        Value Evaluate(const SheetInterface& sheet) const override {
            try {
                auto lmbd = [&](const Position& pos){
                    if (sheet.GetCell(pos) != nullptr) {
                        CellInterface::Value value = sheet.GetCell(pos)->GetValue();
                        if (std::holds_alternative<double>(value)) {
                            return std::get<double>(value);
                        }
                        if (std::holds_alternative<std::string>(value)) {
                            std::string str = std::get<std::string>(value);
                            auto has_alpha = [&](const std::string& s){
                                for (const auto& c : s) {
                                    if (std::isalpha(c)) {
                                        return true;
                                        break;
                                    }
                                }
                                return false;
                            };
                            if (!str.empty() && has_alpha(str)) {
                                throw FormulaError(FormulaError::Category::Value);
                            } else if (!str.empty() && !has_alpha(str)) {
                                try {
                                    return std::stod(str);
                                } catch (...) {
                                    throw FormulaError(FormulaError::Category::Value);
                                }
                            }
                        }
                        if (std::holds_alternative<FormulaError>(value)) {
                            throw std::get<FormulaError>(value);
                        }
                    }
                    return .0;
                };
                return ast_.Execute(lmbd);
            } catch (const FormulaError& ferr) {
                return ferr;
            }
        }

        std::string GetExpression() const override {
            std::ostringstream ss;
            ast_.PrintFormula(ss);
            return ss.str();
        } 

        std::vector<Position> GetReferencedCells() const override {
            std::forward_list<Position> cells = ast_.GetCells();
            cells.sort();
            cells.unique();
            std::vector<Position> c(cells.begin(), cells.end());
            return c;
        }

    private:
        FormulaAST ast_;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}