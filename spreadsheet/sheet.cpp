#include "cell.h"
#include "common.h"
#include "sheet.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
 
using namespace std::literals;
 
Sheet::~Sheet() {}

// Устанавливает содержимое ячейки
void Sheet::SetCell(Position pos, std::string text) 
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("Invalid position");
    }

    cells_.resize(std::max(pos.row + 1, static_cast<int>(cells_.size())));
    cells_[pos.row].resize(std::max(pos.col + 1, static_cast<int>(cells_[pos.row].size())));
    
    if (cells_[pos.row][pos.col].get() == nullptr)
    {
        cells_[pos.row][pos.col] = std::make_unique<Cell>(*this);
    }
    
    cells_[pos.row][pos.col]->Set(std::move(text)); 
}

// Универсальный геттер для константного и неконстантного GetCell()
const Cell* Sheet::CellGetter(Position pos) const 
{
    if (!pos.IsValid()) 
    {
        throw InvalidPositionException("Invalid position");
    } 

    if (IsCellValid(pos.row, pos.col)) 
    {    
        if (cells_[pos.row][pos.col]) 
        {
            return cells_[pos.row][pos.col].get();
        }
    }
    
    return nullptr;
}

// Возвращает указатель на ячейку (неконстантный метод)
Cell* Sheet::GetCell(Position pos) 
{
    return const_cast<Cell*>(CellGetter(pos));
}

// Возвращает указатель на ячейку (константный метод)
const Cell* Sheet::GetCell(Position pos) const
{
    return CellGetter(pos);
}

// Очищает содержимое ячейки
void Sheet::ClearCell(Position pos) 
{    
    if (!pos.IsValid()) 
    {    
        throw InvalidPositionException("Invalid position");
    }

    if (IsCellValid(pos.row, pos.col)) 
    {    
        if (cells_[pos.row][pos.col]) 
        {
            cells_[pos.row][pos.col]->Clear();
            
            if (!cells_[pos.row][pos.col]->IsReferenced()) 
            {
                cells_[pos.row][pos.col].reset();
            }
        }
    }
}

// Проверяет валидность ячейки
bool Sheet::IsCellValid(int row, int col) const
{
    return row < static_cast<int>(cells_.size()) && col < static_cast<int>(cells_[row].size());
}

// Возвращает размер области печати (количество строк и столбцов)
Size Sheet::GetPrintableSize() const 
{    
    Size size { 0, 0 };
    
    for (int row = 0; row < static_cast<int>(cells_.size()); row++)
    {
        for (int col = static_cast<int>(cells_[row].size()) - 1; col >= 0; col--)
        {
            if (cells_[row][col] && !cells_[row][col]->GetText().empty()) 
            {
                size.rows = std::max(size.rows, row + 1);
                size.cols = std::max(size.cols, col + 1);
            }
        }
    }
    
    return size;
}

// Выводит значение ячейки
void Sheet::PrintValue(const Cell* cell, std::ostream& output) const
{
    auto value = cell->GetValue();
    std::visit([&output](const auto& obj) { output << obj; }, value);
}

// Выводит текст ячейки
void Sheet::PrintText(const Cell* cell, std::ostream& output) const
{
    output << cell->GetText();
}

// Выводит содержимое таблицы (текст или значения)
void Sheet::Print(std::ostream& output, bool value) const 
{
    Size size = GetPrintableSize();

    for (int row = 0; row < size.rows; row++) 
    {
        for (int col = 0; col < size.cols; col++)
        {
            if (col > 0) 
            {
                output << "\t";
            }

            if (IsCellValid(row, col)) 
            {
                if (cells_[row][col]) 
                {
                    // Печатаем значение или текст в зависимости от флага value.
                    // Если value == true - печатаем значение
                    if (value) 
                    {
                        PrintValue(cells_[row][col].get(), output);
                    } 
                    
                    // Иначе value == false, значит это text - печатаем текст
                    else 
                    {
                        PrintText(cells_[row][col].get(), output);
                    }
                }
            }
        }
        
        output << "\n";
    }
}

void Sheet::PrintValues(std::ostream& output) const 
{
    Print(output, true);
}

void Sheet::PrintTexts(std::ostream& output) const 
{
    Print(output, false);
}
 
std::unique_ptr<SheetInterface> CreateSheet() 
{
    return std::make_unique<Sheet>();
}