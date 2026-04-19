#pragma once
#include "Board.h"
#include "Evaluate.h"
#include "TranspositionTable.h"
#include <chrono>
#include <vector>
struct SearchInfo {
    int nodes;
    int depth;
    int seldepth;
    double time;
    Move bestMove;
    int score;
    bool stopped;
    SearchInfo() : nodes(0), depth(0), seldepth(0), time(0.0),
                   score(0), stopped(false) {}
};
struct SearchLimits {
    int maxDepth;
    int maxTime;  // milliseconds
    int maxNodes;
    SearchLimits() : maxDepth(64), maxTime(5000), maxNodes(1000000) {}
};
class SearchAdvanced {
public:
    SearchAdvanced();
    ~SearchAdvanced();
    Move findBestMove(Board& board, int team, SearchLimits limits = SearchLimits());
    void stop();
    SearchInfo getInfo() const { return info; }
private:
    Evaluate evaluator;
    TranspositionTable tt;
    SearchInfo info;
    SearchLimits limits;
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    // Killer moves (2 per ply)
    Move killerMoves[64][2];
    // History heuristic
    int history[2][64][64];  // [color][from][to]
    // Search functions
    int iterativeDeepening(Board& board, int team);
    int alphaBeta(Board& board, int team, int depth, int ply,
                  int alpha, int beta, bool nullMove);
    int quiescence(Board& board, int team, int alpha, int beta, int ply);
    // Move ordering
    void orderMoves(std::vector<Move>& moves, Board& board, Move ttMove, int ply);
    int getMoveScore(const Move& move, Board& board, Move ttMove, int ply);
    // Helper functions
    bool shouldStop();
    int mateScore(int ply);
    bool isCapture(Board& board, const Move& move);
    int mvvLva(Board& board, const Move& move);
    // Null move pruning
    bool canDoNullMove(Board& board, int team);
    void clearHistory();
    void updateKillers(Move move, int ply);
    void updateHistory(Move move, int team, int depth);
};