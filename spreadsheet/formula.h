#pragma once

#include "common.h"

#include <memory>

// Формула, позволяющая вычислять и обновлять арифметическое выражение.
// Поддерживаемые возможности:
// * Простые бинарные операции и числа, скобки: 1+2*3, 2.5*(2+3.5/7)
// * Значения ячеек в качестве переменных: A1+B2*C3
// Ячейки, указанные в формуле, могут быть как формулами, так и текстом. Если это
// текст, но он представляет число, тогда его нужно трактовать как число. Пустая
// ячейка или ячейка с пустым текстом трактуется как число ноль.
class FormulaInterface 
{
    public:
    
        // Возвращает вычисленное значение формулы либо ошибку
        using Value = std::variant<double, FormulaError>;

        virtual ~FormulaInterface() = default;
        // Обратите внимание, что в метод Evaluate() ссылка на таблицу передаётся 
        // в качестве аргумента.
        virtual Value Evaluate(const SheetInterface& sheet) const = 0;
        virtual std::string GetExpression() const = 0;
        virtual std::vector<Position> GetReferencedCells() const = 0;
};

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression);