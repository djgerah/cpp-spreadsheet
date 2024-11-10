#include "formula.h"
#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

FormulaError::FormulaError(Category category) 
    : category_(category) {}

FormulaError::Category FormulaError::GetCategory() const 
{
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const 
{
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const 
{
    switch (category_) 
    {
        case FormulaError::Category::Ref:
            return "#REF!"sv;

        case FormulaError::Category::Value:
            return "#VALUE!"sv;

        case FormulaError::Category::Arithmetic:
            return "#ARITHM!"sv;
    }

    return "";
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) 
{
    return output << fe.ToString();
}

namespace 
{
    class Formula : public FormulaInterface 
    {
        public:

            explicit Formula(std::string expression)
                try : ast_(ParseFormulaAST(expression)) 
                    {}

                catch (const std::exception& e) 
                {
                    std::throw_with_nested(FormulaException(e.what()));
                }

            // Возвращает вычисленное значение формулы для переданного листа либо ошибку.
            // Если вычисление какой-то из указанных в формуле ячеек приводит к ошибке, то
            // возвращается именно эта ошибка. Если таких ошибок несколько, возвращается любая.
            Value Evaluate(const SheetInterface& sheet) const override 
            {
                const std::function<double(Position)> args = [&sheet](const Position p)->double 
                {
                    if (!p.IsValid()) throw FormulaError(FormulaError::Category::Ref);

                    const auto* cell = sheet.GetCell(p);

                    if (!cell) 
                    {
                        return 0;
                    }

                    if (std::holds_alternative<double>(cell->GetValue())) 
                    {
                        return std::get<double>(cell->GetValue());
                    }

                    if (std::holds_alternative<std::string>(cell->GetValue())) 
                    {
                        auto value = std::get<std::string>(cell->GetValue());
                        double result = 0;

                        if (!value.empty()) 
                        {
                            std::istringstream in(value);

                            if (!(in >> result) || !in.eof()) 
                            {
                                throw FormulaError(FormulaError::Category::Value);
                            }
                        }

                        return result;
                    }

                    throw FormulaError(std::get<FormulaError>(cell->GetValue()));
                };

                try 
                {
                    return ast_.Execute(args);
                }

                catch (FormulaError& e) 
                {
                    return e;
                }
            }
            
            // Возвращает список ячеек, которые непосредственно задействованы в вычислении
            // формулы. Список отсортирован по возрастанию и не содержит повторяющихся ячеек
            std::vector<Position> GetReferencedCells() const override 
            {
                std::vector<Position> cells;

                for (auto cell : ast_.GetCells()) 
                {
                    if (cell.IsValid()) 
                    {
                        cells.push_back(cell);
                    }
                }

                cells.resize(std::unique(cells.begin(), cells.end()) - cells.begin());

                return cells;
            }

            // Возвращает выражение, которое описывает формулу.
            // Не содержит пробелов и лишних скобок.
            std::string GetExpression() const override 
            {
                std::ostringstream out;
                ast_.PrintFormula(out);

                return out.str();
            }

        private:

            const FormulaAST ast_;
    };
}  // end of namespace

// Парсит переданное выражение и возвращает объект класса Formula.
// Бросает FormulaException в случае, если формула синтаксически некорректна
std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) 
{
    try 
    {
        return std::make_unique<Formula>(std::move(expression));
    }
    
    catch (...) 
    {
        throw FormulaException("Formula exception");
    }
}