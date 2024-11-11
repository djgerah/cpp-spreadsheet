#pragma once
 
#include "cell.h"
#include "common.h"
 
#include <functional>
 
class Sheet : public SheetInterface 
{
    public:

        ~Sheet();
    
        void SetCell(Position pos, std::string text) override;
        const Cell* GetCell(Position pos) const override;
        Cell* GetCell(Position pos) override;
        void ClearCell(Position pos) override;
        bool IsPosValid(Position pos) const;
        bool IsCellValid(int row, int col) const;
        Size GetPrintableSize() const override;
    
        void PrintValues(std::ostream& output) const override;
        void PrintTexts(std::ostream& output) const override;
    
    private:
        // Можете дополнить ваш класс нужными полями и методами
        const Cell* CellGetter(Position pos) const;
        void Print(std::ostream& output, bool value) const;
        void PrintValue(const Cell* cell, std::ostream& output) const;
        void PrintText(const Cell* cell, std::ostream& output) const;

        std::vector<std::vector<std::unique_ptr<Cell>>> cells_;
};