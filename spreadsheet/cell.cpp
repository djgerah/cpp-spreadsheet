#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>

// Реализуйте следующие методы
Cell::Cell(SheetInterface& sheet)
    : sheet_(sheet) 
    {
        impl_ = std::make_unique<EmptyImpl>();
    }

Cell::~Cell() {}

void Cell::Set(std::string text) 
{   
    // Сохраняем старый текст для отката в случае ошибки
    std::string old_text = this->GetText();

    // Если значение text отличается от установленного в ячейке ранее
    if (text != old_text)
    {
        if (text.empty()) 
        {
            // Если текст пустой, устанавливаем пустую реализацию
            impl_ = std::make_unique<EmptyImpl>();
        }

        else if (text.size() > 1 && text[0] == FORMULA_SIGN)
        { 
            // Если текст начинается с символа формулы, создаем реализацию формулы
            impl_ = std::make_unique<FormulaImpl>(std::move(text), sheet_);
        }

        else 
        {
            // В противном случае создаем реализацию текста
            impl_ = std::make_unique<TextImpl>(std::move(text));
        }

        if (IsCircularDependency(*impl_)) 
        {
            // В случае циклической зависимости возвращаем старый текст
            this->Set(std::move(old_text));

            throw CircularDependencyException("Circular Dependency");
        }
        
        UpdateDependence();
        InvalidateCache();
    }
}

// Проверяет наличие циклической зависимости в ячейках
bool Cell::IsCircularDependency(const Impl& impl) const 
{
    if (impl.GetReferencedCells().empty()) 
    {
        return false;
    }

    std::unordered_set<const Cell*> referenced_cells;

    // Собираем все ячейки, на которые ссылается текущая ячейка
    for (const auto& cell_pos : impl.GetReferencedCells()) 
    {
        referenced_cells.insert(dynamic_cast<Cell*>(sheet_.GetCell(cell_pos)));
    }

    // Проверяем на циклическую зависимость с помощью обхода в глубину
    // Список для проверки начинается с текущей ячейки
    std::vector<const Cell*> check_list;
    check_list.push_back(this);
    // Ячейки, которые уже проверены
    std::unordered_set<const Cell*> checked_cells;

    while (!check_list.empty()) 
    {
        const Cell* current_cell = check_list.back();

        // Если текущая ячейка была найдена - есть циклическая зависимость
        if (referenced_cells.find(current_cell) != referenced_cells.end()) 
        {
            return true;
        }

        // Помечаем текущую ячейку как проверенную
        checked_cells.insert(current_cell);
        check_list.pop_back();

        // Цикл по ячейкам, на которые ссылается текущая, добавляем их в список
        for (const Cell* cell : current_cell->referenced_to_) 
        {
            if (checked_cells.find(cell) == checked_cells.end()) 
            {
                check_list.push_back(cell);
            }
        }
    }

    return false;
}

// Обновляет зависимости текущей ячейки
void Cell::UpdateDependence()
{
    for (Cell* cells : referenced_by_) 
    {
        // Удаляем текущую ячейку из зависимостей других ячеек
        cells->referenced_to_.erase(this);
    }

    // Очищаем список зависимых ячеек
    referenced_by_.clear();

    for (const auto& cell_pos : impl_->GetReferencedCells()) 
    {
        Cell* cell = dynamic_cast<Cell*>(sheet_.GetCell(cell_pos));
    
        if (!cell) 
        {
            // Если целевая ячейка не существует, создаем пустую ячейку
            sheet_.SetCell(cell_pos, EMPTY);
            cell = dynamic_cast<Cell*>(sheet_.GetCell(cell_pos));
        }

        // Добавляем целевую ячейку в список зависимых
        referenced_by_.insert(cell);
        // Добавляем текущую ячейку в список зависимостей целевой ячейки
        cell->referenced_to_.insert(this);
    }
} 

// Инвалидирует кэш значений для текущей и зависимых ячеек
void Cell::InvalidateCache() 
{
    if (impl_->IsCacheValid()) 
    {
        impl_->InvalidateCache();

        for (Cell* cells : referenced_to_) 
        {
            cells->InvalidateCache();
        }
    }
}

// Очищает содержимое ячейки
void Cell::Clear() 
{
    // impl_ = std::make_unique<EmptyImpl>();
    // Для очистки используем Cell::Set("ПУСТАЯ СТРОКА")
    Set(EMPTY);
}

// Возвращает значение текущей ячейки
Cell::Value Cell::GetValue() const 
{
    return impl_->GetValue();
}

// Возвращает текстовое представление текущей ячейки
std::string Cell::GetText() const 
{
    return impl_->GetText();
}

// Возвращает список ячеек, на которые ссылается текущая ячейка
std::vector<Position> Cell::GetReferencedCells() const 
{
    return impl_->GetReferencedCells();
}

// Проверяет, ссылается ли текущая ячейка на другие
bool Cell::IsReferenced() const 
{
    return !referenced_to_.empty();
}