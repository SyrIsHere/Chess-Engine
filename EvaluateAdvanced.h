#pragma once
#include "Board.h"
class EvaluateAdvanced {
public:
    int evaluate(Board& board);
    
private:
    // Material values
    static constexpr int PAWN_VALUE = 100;
    static constexpr int KNIGHT_VALUE = 320;
    static constexpr int BISHOP_VALUE = 330;
    static constexpr int ROOK_VALUE = 500;
    static constexpr int QUEEN_VALUE = 900;
    // Piece-square tables
    static const int pawnTable[64];
    static const int knightTable[64];
    static const int bishopTable[64];
    static const int rookTable[64];
    static const int queenTable[64];
    static const int kingMiddleGameTable[64];
    static const int kingEndGameTable[64];
    // Evaluation components
    int evaluateMaterial(Board& board);
    int evaluatePieceSquareTables(Board& board);
    int evaluatePawnStructure(Board& board);
    int evaluateKingSafety(Board& board);
    int evaluateMobility(Board& board);
    int evaluateBishopPair(Board& board);
    int evaluateRooksOnOpenFiles(Board& board);
    // Helper functions
    bool isEndgame(Board& board);
    int getPhase(Board& board);
    int getPieceValue(int piece);
    int mirrorSquare(int square);
};