#include "EvaluateAdvanced.h"
#include <algorithm>
// Piece-Square Tables (from white's perspective)
const int EvaluateAdvanced::pawnTable[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
     50,  50,  50,  50,  50,  50,  50,  50,
     10,  10,  20,  30,  30,  20,  10,  10,
      5,   5,  10,  25,  25,  10,   5,   5,
      0,   0,   0,  20,  20,   0,   0,   0,
      5,  -5, -10,   0,   0, -10,  -5,   5,
      5,  10,  10, -20, -20,  10,  10,   5,
      0,   0,   0,   0,   0,   0,   0,   0
};
const int EvaluateAdvanced::knightTable[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20,   0,   0,   0,   0, -20, -40,
    -30,   0,  10,  15,  15,  10,   0, -30,
    -30,   5,  15,  20,  20,  15,   5, -30,
    -30,   0,  15,  20,  20,  15,   0, -30,
    -30,   5,  10,  15,  15,  10,   5, -30,
    -40, -20,   0,   5,   5,   0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
};
const int EvaluateAdvanced::bishopTable[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   5,  10,  10,   5,   0, -10,
    -10,   5,   5,  10,  10,   5,   5, -10,
    -10,   0,  10,  10,  10,  10,   0, -10,
    -10,  10,  10,  10,  10,  10,  10, -10,
    -10,   5,   0,   0,   0,   0,   5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};
const int EvaluateAdvanced::rookTable[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
      5,  10,  10,  10,  10,  10,  10,   5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
      0,   0,   0,   5,   5,   0,   0,   0
};
const int EvaluateAdvanced::queenTable[64] = {
    -20, -10, -10,  -5,  -5, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   5,   5,   5,   5,   0, -10,
     -5,   0,   5,   5,   5,   5,   0,  -5,
      0,   0,   5,   5,   5,   5,   0,  -5,
    -10,   5,   5,   5,   5,   5,   0, -10,
    -10,   0,   5,   0,   0,   0,   0, -10,
    -20, -10, -10,  -5,  -5, -10, -10, -20
};
const int EvaluateAdvanced::kingMiddleGameTable[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
     20,  20,   0,   0,   0,   0,  20,  20,
     20,  30,  10,   0,   0,  10,  30,  20
};
const int EvaluateAdvanced::kingEndGameTable[64] = {
    -50, -40, -30, -20, -20, -30, -40, -50,
    -30, -20, -10,   0,   0, -10, -20, -30,
    -30, -10,  20,  30,  30,  20, -10, -30,
    -30, -10,  30,  40,  40,  30, -10, -30,
    -30, -10,  30,  40,  40,  30, -10, -30,
    -30, -10,  20,  30,  30,  20, -10, -30,
    -30, -30,   0,   0,   0,   0, -30, -30,
    -50, -30, -30, -30, -30, -30, -30, -50
};
int EvaluateAdvanced::mirrorSquare(int square) {
    return square ^ 56;  // Flip rank
}
int EvaluateAdvanced::getPieceValue(int piece) {
    int absP = std::abs(piece);
    switch (absP) {
        case 1: return PAWN_VALUE;
        case 2: return KNIGHT_VALUE;
        case 3: return BISHOP_VALUE;
        case 4: return ROOK_VALUE;
        case 5: return QUEEN_VALUE;
        default: return 0;
    }
}
int EvaluateAdvanced::evaluate(Board& board) {
    int score = 0;
    score += evaluateMaterial(board);
    score += evaluatePieceSquareTables(board);
    score += evaluatePawnStructure(board);
    score += evaluateKingSafety(board);
    score += evaluateMobility(board);
    score += evaluateBishopPair(board);
    score += evaluateRooksOnOpenFiles(board);
    return score;
}
int EvaluateAdvanced::evaluateMaterial(Board& board) {
    int score = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int piece = board.state[r][c];
            if (piece != 0 && piece != 7 && piece != -7) {
                int value = getPieceValue(piece);
                score += (piece > 0) ? value : -value;
            }
        }
    }
    return score;
}
int EvaluateAdvanced::evaluatePieceSquareTables(Board& board) {
    int score = 0;
    bool endgame = isEndgame(board);
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int piece = board.state[r][c];
            if (piece == 0 || piece == 7 || piece == -7) continue;
            int square = r * 8 + c;
            int absP = std::abs(piece);
            bool isWhite = piece > 0;
            int sq = isWhite ? square : mirrorSquare(square);
            int bonus = 0;
            switch (absP) {
                case 1: bonus = pawnTable[sq]; break;
                case 2: bonus = knightTable[sq]; break;
                case 3: bonus = bishopTable[sq]; break;
                case 4: bonus = rookTable[sq]; break;
                case 5: bonus = queenTable[sq]; break;
                case 6:
                    bonus = endgame ? kingEndGameTable[sq] : kingMiddleGameTable[sq];
                    break;
            }
            score += isWhite ? bonus : -bonus;
        }
    }
    return score;
}
int EvaluateAdvanced::evaluatePawnStructure(Board& board) {
    int score = 0;
    // Doubled pawns penalty
    for (int file = 0; file < 8; file++) {
        int whitePawns = 0, blackPawns = 0;
        for (int rank = 0; rank < 8; rank++) {
            int piece = board.state[rank][file];
            if (piece == 1) whitePawns++;
            if (piece == -1) blackPawns++;
        }
        if (whitePawns > 1) score -= 20 * (whitePawns - 1);
        if (blackPawns > 1) score += 20 * (blackPawns - 1);
    }
    // Isolated pawns penalty
    for (int file = 0; file < 8; file++) {
        bool hasWhitePawn = false, hasBlackPawn = false;
        bool hasWhiteNeighbor = false, hasBlackNeighbor = false;
        for (int rank = 0; rank < 8; rank++) {
            if (board.state[rank][file] == 1) hasWhitePawn = true;
            if (board.state[rank][file] == -1) hasBlackPawn = true;
        }
        if (file > 0) {
            for (int rank = 0; rank < 8; rank++) {
                if (board.state[rank][file - 1] == 1) hasWhiteNeighbor = true;
                if (board.state[rank][file - 1] == -1) hasBlackNeighbor = true;
            }
        }
        if (file < 7) {
            for (int rank = 0; rank < 8; rank++) {
                if (board.state[rank][file + 1] == 1) hasWhiteNeighbor = true;
                if (board.state[rank][file + 1] == -1) hasBlackNeighbor = true;
            }
        }
        if (hasWhitePawn && !hasWhiteNeighbor) score -= 15;
        if (hasBlackPawn && !hasBlackNeighbor) score += 15;
    }
    // Passed pawns bonus
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int piece = board.state[r][c];
            if (piece == 1) {
                bool passed = true;
                for (int rank = r - 1; rank >= 0; rank--) {
                    for (int file = std::max(0, c - 1); file <= std::min(7, c + 1); file++) {
                        if (board.state[rank][file] == -1) {
                            passed = false;
                            break;
                        }
                    }
                    if (!passed) break;
                }
                if (passed) score += 20 + (7 - r) * 10;
            } else if (piece == -1) {
                bool passed = true;
                for (int rank = r + 1; rank < 8; rank++) {
                    for (int file = std::max(0, c - 1); file <= std::min(7, c + 1); file++) {
                        if (board.state[rank][file] == 1) {
                            passed = false;
                            break;
                        }
                    }
                    if (!passed) break;
                }
                if (passed) score -= 20 + r * 10;
            }
        }
    }
    return score;
}
int EvaluateAdvanced::evaluateKingSafety(Board& board) {
    int score = 0;
    // Find kings
    int whiteKingRow = -1, whiteKingCol = -1;
    int blackKingRow = -1, blackKingCol = -1;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (board.state[r][c] == 6) {
                whiteKingRow = r;
                whiteKingCol = c;
            } else if (board.state[r][c] == -6) {
                blackKingRow = r;
                blackKingCol = c;
            }
        }
    }
    
    // Pawn shield bonus
    if (whiteKingRow >= 0) {
        int shield = 0;
        if (whiteKingRow == 7) {
            if (whiteKingRow - 1 >= 0) {
                for (int dc = -1; dc <= 1; dc++) {
                    int c = whiteKingCol + dc;
                    if (c >= 0 && c < 8 && board.state[whiteKingRow - 1][c] == 1) {
                        shield += 10;
                    }
                }
            }
        }
        score += shield;
    }
    if (blackKingRow >= 0) {
        int shield = 0;
        if (blackKingRow == 0) {
            if (blackKingRow + 1 < 8) {
                for (int dc = -1; dc <= 1; dc++) {
                    int c = blackKingCol + dc;
                    if (c >= 0 && c < 8 && board.state[blackKingRow + 1][c] == -1) {
                        shield += 10;
                    }
                }
            }
        }
        score -= shield;
    }
    return score;
}
int EvaluateAdvanced::evaluateMobility(Board& board) {
    int whiteMobility = board.getAllLegalMoves(1).size();
    int blackMobility = board.getAllLegalMoves(-1).size();
    return (whiteMobility - blackMobility) * 2;
}
int EvaluateAdvanced::evaluateBishopPair(Board& board) {
    int whiteBishops = 0, blackBishops = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (board.state[r][c] == 3) whiteBishops++;
            if (board.state[r][c] == -3) blackBishops++;
        }
    }
    int score = 0;
    if (whiteBishops >= 2) score += 30;
    if (blackBishops >= 2) score -= 30;
    return score;
}
int EvaluateAdvanced::evaluateRooksOnOpenFiles(Board& board) {
    int score = 0;
    for (int file = 0; file < 8; file++) {
        bool hasPawn = false;
        bool hasWhiteRook = false, hasBlackRook = false;
        int whiteRookRank = -1, blackRookRank = -1;
        for (int rank = 0; rank < 8; rank++) {
            int piece = board.state[rank][file];
            if (piece == 1 || piece == -1) hasPawn = true;
            if (piece == 4) {
                hasWhiteRook = true;
                whiteRookRank = rank;
            }
            if (piece == -4) {
                hasBlackRook = true;
                blackRookRank = rank;
            }
        }
        if (!hasPawn) {
            if (hasWhiteRook) score += 20;
            if (hasBlackRook) score -= 20;
        }
    }
    return score;
}
bool EvaluateAdvanced::isEndgame(Board& board) {
    int queens = 0, minorPieces = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int piece = std::abs(board.state[r][c]);
            if (piece == 5) queens++;
            if (piece == 2 || piece == 3) minorPieces++;
        }
    }
    return queens == 0 || (queens == 2 && minorPieces <= 2);
}
int EvaluateAdvanced::getPhase(Board& board) {
    int phase = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int piece = std::abs(board.state[r][c]);
            switch (piece) {
                case 2: case 3: phase += 1; break;
                case 4: phase += 2; break;
                case 5: phase += 4; break;
            }
        }
    }
    return std::min(phase, 24);
}