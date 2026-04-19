#pragma once
#include "Board.h"
#include "Evaluate.h"

class Search {
public:
    Search();
    std::pair<double, Move> minmax(Board& board, int team, int depth, double alpha, double beta);

private:
    Evaluate evaluator;
    Board copyBoard(Board& board);
    int moveOrderScore(Board& board, const Move& move);
};
