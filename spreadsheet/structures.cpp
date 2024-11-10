#include "common.h"
#include "sheet.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <tuple>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = { -1, -1 };

bool Position::operator==(const Position rhs) const 
{
    return std::tie(row, col) == std::tie(rhs.row, rhs.col);
}

bool Position::operator<(const Position rhs) const 
{
    return std::tie(row, col) < std::tie(rhs.row, rhs.col);
}

/*
    bool IsValid() проверяет валидность позиции, то есть что ячейка (row, col) не выходит за ограничения ниже 
    и что значения полей row и col неотрицательны. Position::NONE невалидна
*/

bool Position::IsValid() const 
{
    return ((row < MAX_ROWS && row >= 0) && (col < MAX_COLS && col >= 0));
}

// Преобразует позицию в строку ({ 0, 0 } в A1 и т.п.)
std::string Position::ToString() const 
{   
    std::string result;

    if (IsValid()) 
    {   
        int columns = col;

        // Преобразуем номер столбца в буквы (например, 0 -> A, 1 -> B и т.д.)
        while (columns >= 0) 
        {
            // Добавляем букву в начало строки
            result.insert(result.begin(), 'A' + columns % LETTERS);
            // Переходим к следующему разряду столбца
            columns = columns / LETTERS - 1;
        }

        result += std::to_string(row + 1);
    }
    
    else 
    {
        result = "";
    }

    return result;
}

// Преобразует строку в позицию (A1 в { 0, 0 } и т.п.)
Position Position::FromString(std::string_view str) 
{   
    // Находим первую небуквенную часть строки
    auto pos = std::find_if(str.begin(), str.end(), [](const char c) 
                            {
                                return !(std::isalpha(c) && std::isupper(c));
                            });

    auto letters = str.substr(0, pos - str.begin());
    auto digits = str.substr(pos - str.begin());
 
    if ((letters.empty() || digits.empty()) || (letters.size() > MAX_POS_LETTER_COUNT) || (!std::isdigit(digits[0]))) 
    {
        return Position::NONE;
    }
    
    int row = 0;
    // Создаем поток для чтения чисел
    std::istringstream row_in { std::string(digits) };
        
    if (!(row_in >> row) || !row_in.eof()) 
    {
        // Если не удалось прочитать строку как число, возвращаем невалидную позицию
        return Position::NONE;
    }
 
    int col = 0;

    // Преобразуем буквы в номер столбца
    for (char ch : letters) 
    {
        // Умножаем на количество букв для перехода к следующему разряду
        col *= LETTERS;
        // Преобразуем символ в индекс столбца (A -> 1, B -> 2 и т.д.)
        col += ch - 'A' + 1;
    }

    return { row - 1, col - 1 };
}

bool Size::operator==(Size rhs) const 
{
    return cols == rhs.cols && rows == rhs.rows;
}