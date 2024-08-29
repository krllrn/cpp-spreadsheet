#include "common.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <sstream>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = {-1, -1};

// Реализуйте методы:
bool Position::operator==(const Position rhs) const {
    return row == rhs.row && col == rhs.col;
}

bool Position::operator<(const Position rhs) const {
    return (row < rhs.row && col < rhs.col) || 
            (row < rhs.row && col == rhs.col) || 
            (row == rhs.row && col < rhs.col);
}

bool Position::IsValid() const {
    return !((row < 0 || col < 0) || (row >= MAX_ROWS || col >= MAX_COLS));
}

int GetAlphabetCount(std::string_view str) {
    int i = 0;
    for (const char& c : str) {
        if (isdigit(c)) {
            break;
        }
        ++i;
    }

    return i;
}

int GetAlphabetCount(int col) {
    int alphabet_count = 1;
    if (col - 26 >= 0) {
        if (col - 26 < 26) {
            alphabet_count = 2;
        } else {
            while (26 <= col - std::pow(26, alphabet_count)) {
                ++alphabet_count;
            }
        }    
    }

    return alphabet_count;
}

std::string Position::ToString() const {
    if (!IsValid()) {
        return "";
    }

    std::string col_s;
    int alphabet_count = GetAlphabetCount(col);
    int tmp_col = col;

    for (int j = 1; j <= alphabet_count; ++j) {
        char c = (tmp_col % 26) + 65;
        col_s = c + col_s;
        tmp_col = (tmp_col / 26) - 1;
    }

    col_s += std::to_string(row + 1);

    return col_s;
}

bool SubstrIsValid(std::string_view row, std::string_view col) {
    bool result = true;
    bool col_is_alpha = std::all_of(col.begin(), col.end(), [](const char c){
        return std::isalpha(c);
    });
    
    bool row_is_digit = std::all_of(row.begin(), row.end(), [](const char c){
        return std::isdigit(c);
    });
    
    bool col_not_valid = std::any_of(col.begin(), col.end(), [](const char c){
        return !std::isalpha(c) || std::islower(c);
    });

    
    try
    {
       [[maybe_unused]] int r = std::stoi(row.data());
    }
    catch (...) {
        result = false;
    }
    
    if (col.empty() || row.empty() || !col_is_alpha || !row_is_digit || col_not_valid) {
        result = false;
    }

    return result;
}

Position Position::FromString(std::string_view str) {
    int alphabet_count = GetAlphabetCount(str);

    std::string col_s = (std::string)str.substr(0, alphabet_count);
    std::string row_s = (std::string)str.substr(alphabet_count, str.size());

    if (!SubstrIsValid(row_s, col_s)) {
        return NONE;
    }

    int row = std::stoi(row_s) - 1;
    int col = 0;
    for(int i = 0; i < alphabet_count; ++i) {
        col += ((col_s[i]) - 64) * std::pow(26, (alphabet_count - 1) - i);
    }

    Position pos{row, col - 1};
    if (pos.IsValid()) {
        return pos;
    } else {
        return NONE;
    }
}

bool Size::operator==(Size rhs) const {
    return rows == rhs.rows && cols == rhs.cols;
}

bool Position::EqualTo::operator()(const Position& lhs, const Position& rhs) const {
        return (lhs.row == rhs.row && lhs.col == rhs.col);
}

size_t Position::Hasher::operator()(const Position& pos) const {
    size_t hash_s = std::hash<int>{}(pos.row) * 37
        + std::hash<int>{}(pos.col) * 37 * 37 * 37;
    return hash_s;
}

FormulaError::FormulaError(FormulaError::Category category) 
: category_(category)
{
}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return this->category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_)
    {
    case FormulaError::Category::Ref:
        return "Ref";
        break;
    case FormulaError::Category::Value:
        return "Value";
        break;
    case FormulaError::Category::Arithmetic:
        return "Arithmetic";
        break;
    default:
        throw std::invalid_argument("Unknown category!");
    }
}