#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <optional>
#include <unordered_set>

inline const std::string EMPTY = "";

class Cell : public CellInterface 
{
    public:

        Cell(SheetInterface& sheet);
        ~Cell();

        void Set(std::string text);
        void Clear();
        Value GetValue() const override;
        std::string GetText() const override;
        bool IsReferenced() const;
        std::vector<Position> GetReferencedCells() const override;

    private:

        class Impl 
        {
            public:
            
                virtual ~Impl() = default;
                virtual Value GetValue() const = 0;
                virtual std::string GetText() const = 0;
                virtual std::vector<Position> GetReferencedCells() const 
                { 
                    return {}; 
                }
                
                virtual bool IsCacheValid() const 
                { 
                    return true; 
                }
                
                virtual void InvalidateCache() {}
        };

        class EmptyImpl : public Impl 
        {
            public:

                Value GetValue() const override 
                { 
                    return EMPTY; 
                }
                
                std::string GetText() const override 
                { 
                    return EMPTY; 
                }
        };

        class TextImpl : public Impl 
        {
            public:
            
                TextImpl(std::string text) 
                {
                    if (text.empty()) 
                    { 
                        throw std::logic_error("Empty"); 
                    }

                    text_ = text;
                }

                Value GetValue() const override 
                {
                    if (text_[0] == ESCAPE_SIGN) 
                    {
                        return text_.substr(1);
                    }
                    
                    return text_;
                }

                std::string GetText() const override 
                {
                    return text_;
                }

            private:
            
                std::string text_;
        };

        class FormulaImpl : public Impl 
        {
            public:
            
                explicit FormulaImpl(std::string expression, const SheetInterface& sheet)
                    : sheet_(sheet) 
                    {
                        if (expression.empty() || expression[0] != FORMULA_SIGN) 
                        {
                            throw std::logic_error("");
                        }

                            formula_ptr_ = ParseFormula(expression.substr(1));
                    }

                Value GetValue() const override 
                {
                    auto value = formula_ptr_->Evaluate(sheet_);

                    if (!cache_.has_value())
                    {
                        cache_ = value;
                    }

                    if (std::holds_alternative<double>(value))
                    {
                        return std::get<double>(value);
                    }
                    
                    else 
                    {
                        return std::get<FormulaError>(value);
                    }
                }

                std::string GetText() const override 
                {
                    return FORMULA_SIGN + formula_ptr_->GetExpression();
                }

                bool IsCacheValid() const override 
                {
                    return cache_.has_value();
                }

                void InvalidateCache() override 
                {
                    cache_.reset();
                }

                std::vector<Position> GetReferencedCells() const override
                {
                    return formula_ptr_->GetReferencedCells();
                }

            private:
            
                std::unique_ptr<FormulaInterface> formula_ptr_;
                const SheetInterface& sheet_;
                // Если кэш валидный, optional хранит Value
                mutable std::optional<FormulaInterface::Value> cache_;
        };
 
        // Добавьте поля и методы для связи с таблицей, проверки циклических 
        // зависимостей, графа зависимостей и т. д.
        bool IsCircularDependency(const Impl& impl) const;
        void UpdateDependence();
        void InvalidateCache();

        std::unique_ptr<Impl> impl_;
        SheetInterface& sheet_;

        // Контейнер указателей ячеек, на которые ссылается данная ячейка (поиск циклических зависимостей)
        std::unordered_set<Cell*> referenced_to_;
        // Контейнер указателей ячеек, которые ссылаются на данную ячейку (инвалидация кеша)
        std::unordered_set<Cell*> referenced_by_;
};