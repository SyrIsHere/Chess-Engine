#include "Board.h"
#include <algorithm>

Board::Board() {
    state = {{
        {{-4,-2,-3,-5,-6,-3,-2,-4}},
        {{-1,-1,-1,-1,-1,-1,-1,-1}},
        {{ 0, 0, 0, 0, 0, 0, 0, 0}},
        {{ 0, 0, 0, 0, 0, 0, 0, 0}},
        {{ 0, 0, 0, 0, 0, 0, 0, 0}},
        {{ 0, 0, 0, 0, 0, 0, 0, 0}},
        {{ 1, 1, 1, 1, 1, 1, 1, 1}},
        {{ 4, 2, 3, 5, 6, 3, 2, 4}}
    }};
    
    whiteKingMoved = false;
    blackKingMoved = false;
    whiteRookAMoved = false;
    whiteRookHMoved = false;
    blackRookAMoved = false;
    blackRookHMoved = false;
}

void Board::movePiece(int row, int col, int newRow, int newCol, int promotionPiece) {
    int piece = state[row][col];
    int team = (state[row][col] > 0) ? 1 : -1;
    int targetPiece = state[newRow][newCol];

    // White kingside castle
    if (piece == 6 && row == 7 && col == 4 && newRow == 7 && newCol == 6 && state[7][7] == 4) {
        state[row][col] = 0;
        state[newRow][newCol] = piece;
        state[7][5] = 4;
        state[7][7] = 0;
    }
    // White queenside castle
    else if (piece == 6 && row == 7 && col == 4 && newRow == 7 && newCol == 2 && state[7][0] == 4) {
        state[row][col] = 0;
        state[newRow][newCol] = piece;
        state[7][3] = 4;
        state[7][0] = 0;
    }
    // Black kingside castle
    else if (piece == -6 && row == 0 && col == 4 && newRow == 0 && newCol == 6 && state[0][7] == -4) {
        state[row][col] = 0;
        state[newRow][newCol] = piece;
        state[0][5] = -4;
        state[0][7] = 0;
    }
    // Black queenside castle
    else if (piece == -6 && row == 0 && col == 4 && newRow == 0 && newCol == 2 && state[0][0] == -4) {
        state[row][col] = 0;
        state[newRow][newCol] = piece;
        state[0][3] = -4;
        state[0][0] = 0;
    }
    // White en passant capture
    else if (piece == 1 && targetPiece == -7) {
        state[row][col] = 0;
        state[newRow][newCol] = (promotionPiece != 0) ? promotionPiece : piece;
        state[newRow + 1][newCol] = 0;
    }
    // Black en passant capture
    else if (piece == -1 && targetPiece == 7) {
        state[row][col] = 0;
        state[newRow][newCol] = (promotionPiece != 0) ? -promotionPiece : piece;
        state[newRow - 1][newCol] = 0;
    }
    // Normal move
    else {
        state[row][col] = 0;
        if (promotionPiece != 0) {
            state[newRow][newCol] = (team == 1) ? promotionPiece : -promotionPiece;
        } else {
            state[newRow][newCol] = piece;
        }
    }

    // Clear old en passant markers
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (state[i][j] == 7 || state[i][j] == -7) {
                state[i][j] = 0;
            }
        }
    }

    // Set en passant markers
    if (piece == 1 && newRow == row - 2) {
        state[row - 1][col] = 7;
    } else if (piece == -1 && newRow == row + 2) {
        state[row + 1][col] = -7;
    }

    // Track castling rights
    if (piece == 6) whiteKingMoved = true;
    else if (piece == -6) blackKingMoved = true;
    else if (piece == 4 && row == 7 && col == 0) whiteRookAMoved = true;
    else if (piece == 4 && row == 7 && col == 7) whiteRookHMoved = true;
    else if (piece == -4 && row == 0 && col == 0) blackRookAMoved = true;
    else if (piece == -4 && row == 0 && col == 7) blackRookHMoved = true;
}

std::vector<Move> Board::longRangeRecursion(int row, int col, int dirX, int dirY, int team) {
    std::vector<Move> legalMoves;
    int startRow = row;
    int startCol = col;
    int cRow = row + dirX;
    int cCol = col + dirY;

    while (cRow >= 0 && cRow <= 7 && cCol >= 0 && cCol <= 7) {
        int cPiece = state[cRow][cCol];

        if (cPiece == 0) {
            legalMoves.push_back(Move(startRow, startCol, cRow, cCol, 0));
            cRow += dirX;
            cCol += dirY;
        } else {
            if ((team > 0 && cPiece < 0) || (team < 0 && cPiece > 0)) {
                legalMoves.push_back(Move(startRow, startCol, cRow, cCol, 0));
            }
            break;
        }
    }
    
    return legalMoves;
}

void Board::kingCheckCheck(bool& whiteInCheck, bool& blackInCheck,
                           int& whiteKingRow, int& whiteKingCol,
                           int& blackKingRow, int& blackKingCol) {
    whiteInCheck = false;
    blackInCheck = false;
    whiteKingRow = whiteKingCol = blackKingRow = blackKingCol = -1;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (state[row][col] == 6) { // White king
                whiteKingRow = row;
                whiteKingCol = col;
                
                // Check for black pawns
                if (row - 1 >= 0) {
                    if (col + 1 <= 7 && state[row - 1][col + 1] == -1) whiteInCheck = true;
                    if (col - 1 >= 0 && state[row - 1][col - 1] == -1) whiteInCheck = true;
                }
                
                // Check for knights
                int knightMoves[][2] = {{2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2}};
                for (auto& km : knightMoves) {
                    int r = row + km[0], c = col + km[1];
                    if (r >= 0 && r <= 7 && c >= 0 && c <= 7 && state[r][c] == -2) whiteInCheck = true;
                }
                
                // Check diagonals for bishops/queens
                int diagDirs[][2] = {{-1,-1},{-1,1},{1,-1},{1,1}};
                for (auto& dir : diagDirs) {
                    auto moves = longRangeRecursion(row, col, dir[0], dir[1], 1);
                    for (auto& m : moves) {
                        if (state[m.toRow][m.toCol] == -3 || state[m.toRow][m.toCol] == -5) whiteInCheck = true;
                    }
                }
                
                // Check straights for rooks/queens
                int straightDirs[][2] = {{1,0},{-1,0},{0,1},{0,-1}};
                for (auto& dir : straightDirs) {
                    auto moves = longRangeRecursion(row, col, dir[0], dir[1], 1);
                    for (auto& m : moves) {
                        if (state[m.toRow][m.toCol] == -4 || state[m.toRow][m.toCol] == -5) whiteInCheck = true;
                    }
                }
                
                // Check for king
                int kingMoves[][2] = {{1,-1},{1,0},{1,1},{0,-1},{0,1},{-1,-1},{-1,0},{-1,1}};
                for (auto& km : kingMoves) {
                    int r = row + km[0], c = col + km[1];
                    if (r >= 0 && r <= 7 && c >= 0 && c <= 7 && state[r][c] == -6) whiteInCheck = true;
                }
            }
            else if (state[row][col] == -6) { // Black king
                blackKingRow = row;
                blackKingCol = col;
                
                // Check for white pawns
                if (row + 1 <= 7) {
                    if (col + 1 <= 7 && state[row + 1][col + 1] == 1) blackInCheck = true;
                    if (col - 1 >= 0 && state[row + 1][col - 1] == 1) blackInCheck = true;
                }
                
                // Check for knights
                int knightMoves[][2] = {{2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2}};
                for (auto& km : knightMoves) {
                    int r = row + km[0], c = col + km[1];
                    if (r >= 0 && r <= 7 && c >= 0 && c <= 7 && state[r][c] == 2) blackInCheck = true;
                }
                
                // Check diagonals
                int diagDirs[][2] = {{-1,-1},{-1,1},{1,-1},{1,1}};
                for (auto& dir : diagDirs) {
                    auto moves = longRangeRecursion(row, col, dir[0], dir[1], -1);
                    for (auto& m : moves) {
                        if (state[m.toRow][m.toCol] == 3 || state[m.toRow][m.toCol] == 5) blackInCheck = true;
                    }
                }
                
                // Check straights
                int straightDirs[][2] = {{1,0},{-1,0},{0,1},{0,-1}};
                for (auto& dir : straightDirs) {
                    auto moves = longRangeRecursion(row, col, dir[0], dir[1], -1);
                    for (auto& m : moves) {
                        if (state[m.toRow][m.toCol] == 4 || state[m.toRow][m.toCol] == 5) blackInCheck = true;
                    }
                }
                
                // Check for king
                int kingMoves[][2] = {{1,-1},{1,0},{1,1},{0,-1},{0,1},{-1,-1},{-1,0},{-1,1}};
                for (auto& km : kingMoves) {
                    int r = row + km[0], c = col + km[1];
                    if (r >= 0 && r <= 7 && c >= 0 && c <= 7 && state[r][c] == 6) blackInCheck = true;
                }
            }
        }
    }

    if (!whiteInCheck) whiteKingRow = whiteKingCol = -1;
    if (!blackInCheck) blackKingRow = blackKingCol = -1;
}

std::vector<Move> Board::getLegalMoves(int row, int col) {
    int piece = state[row][col];
    std::vector<Move> legalMoves;
    int team = (piece > 0) ? 1 : -1;
    int absP = std::abs(piece);

    // White pawn
    if (piece == 1) {
        int legalRow = row - 1;
        if (legalRow >= 0 && state[legalRow][col] == 0) {
            if (legalRow == 0) {
                legalMoves.push_back(Move(row, col, legalRow, col, 2));
                legalMoves.push_back(Move(row, col, legalRow, col, 3));
                legalMoves.push_back(Move(row, col, legalRow, col, 4));
                legalMoves.push_back(Move(row, col, legalRow, col, 5));
            } else {
                legalMoves.push_back(Move(row, col, legalRow, col, 0));
            }
        }
        
        if (row == 6 && state[row - 2][col] == 0 && state[row - 1][col] == 0) {
            legalMoves.push_back(Move(row, col, row - 2, col, 0));
        }
        
        // Captures
        if (legalRow >= 0) {
            if (col - 1 >= 0 && state[legalRow][col - 1] < 0) {
                if (legalRow == 0) {
                    legalMoves.push_back(Move(row, col, legalRow, col - 1, 2));
                    legalMoves.push_back(Move(row, col, legalRow, col - 1, 3));
                    legalMoves.push_back(Move(row, col, legalRow, col - 1, 4));
                    legalMoves.push_back(Move(row, col, legalRow, col - 1, 5));
                } else {
                    legalMoves.push_back(Move(row, col, legalRow, col - 1, 0));
                }
            }
            if (col + 1 <= 7 && state[legalRow][col + 1] < 0) {
                if (legalRow == 0) {
                    legalMoves.push_back(Move(row, col, legalRow, col + 1, 2));
                    legalMoves.push_back(Move(row, col, legalRow, col + 1, 3));
                    legalMoves.push_back(Move(row, col, legalRow, col + 1, 4));
                    legalMoves.push_back(Move(row, col, legalRow, col + 1, 5));
                } else {
                    legalMoves.push_back(Move(row, col, legalRow, col + 1, 0));
                }
            }
        }
    }
    // Black pawn
    else if (piece == -1) {
        int legalRow = row + 1;
        if (legalRow <= 7 && state[legalRow][col] == 0) {
            if (legalRow == 7) {
                legalMoves.push_back(Move(row, col, legalRow, col, 2));
                legalMoves.push_back(Move(row, col, legalRow, col, 3));
                legalMoves.push_back(Move(row, col, legalRow, col, 4));
                legalMoves.push_back(Move(row, col, legalRow, col, 5));
            } else {
                legalMoves.push_back(Move(row, col, legalRow, col, 0));
            }
        }
        
        if (row == 1 && state[row + 2][col] == 0 && state[row + 1][col] == 0) {
            legalMoves.push_back(Move(row, col, row + 2, col, 0));
        }
        
        // Captures
        if (legalRow <= 7) {
            if (col - 1 >= 0 && state[legalRow][col - 1] > 0) {
                if (legalRow == 7) {
                    legalMoves.push_back(Move(row, col, legalRow, col - 1, 2));
                    legalMoves.push_back(Move(row, col, legalRow, col - 1, 3));
                    legalMoves.push_back(Move(row, col, legalRow, col - 1, 4));
                    legalMoves.push_back(Move(row, col, legalRow, col - 1, 5));
                } else {
                    legalMoves.push_back(Move(row, col, legalRow, col - 1, 0));
                }
            }
            if (col + 1 <= 7 && state[legalRow][col + 1] > 0) {
                if (legalRow == 7) {
                    legalMoves.push_back(Move(row, col, legalRow, col + 1, 2));
                    legalMoves.push_back(Move(row, col, legalRow, col + 1, 3));
                    legalMoves.push_back(Move(row, col, legalRow, col + 1, 4));
                    legalMoves.push_back(Move(row, col, legalRow, col + 1, 5));
                } else {
                    legalMoves.push_back(Move(row, col, legalRow, col + 1, 0));
                }
            }
        }
    }
    // Knight
    else if (absP == 2) {
        int knightMoves[][2] = {{2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2}};
        for (auto& km : knightMoves) {
            int r = row + km[0], c = col + km[1];
            if (r >= 0 && r <= 7 && c >= 0 && c <= 7) {
                if ((team == 1 && state[r][c] <= 0) || (team == -1 && state[r][c] >= 0)) {
                    legalMoves.push_back(Move(row, col, r, c, 0));
                }
            }
        }
    }
    // Bishop
    else if (absP == 3) {
        auto moves = longRangeRecursion(row, col, -1, -1, team);
        legalMoves.insert(legalMoves.end(), moves.begin(), moves.end());
        moves = longRangeRecursion(row, col, -1, 1, team);
        legalMoves.insert(legalMoves.end(), moves.begin(), moves.end());
        moves = longRangeRecursion(row, col, 1, -1, team);
        legalMoves.insert(legalMoves.end(), moves.begin(), moves.end());
        moves = longRangeRecursion(row, col, 1, 1, team);
        legalMoves.insert(legalMoves.end(), moves.begin(), moves.end());
    }
    // Rook
    else if (absP == 4) {
        auto moves = longRangeRecursion(row, col, 1, 0, team);
        legalMoves.insert(legalMoves.end(), moves.begin(), moves.end());
        moves = longRangeRecursion(row, col, -1, 0, team);
        legalMoves.insert(legalMoves.end(), moves.begin(), moves.end());
        moves = longRangeRecursion(row, col, 0, 1, team);
        legalMoves.insert(legalMoves.end(), moves.begin(), moves.end());
        moves = longRangeRecursion(row, col, 0, -1, team);
        legalMoves.insert(legalMoves.end(), moves.begin(), moves.end());
    }
    // Queen
    else if (absP == 5) {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;
                auto moves = longRangeRecursion(row, col, dx, dy, team);
                legalMoves.insert(legalMoves.end(), moves.begin(), moves.end());
            }
        }
    }
    // King
    else if (absP == 6) {
        int kingMoves[][2] = {{1,-1},{1,0},{1,1},{0,-1},{0,1},{-1,-1},{-1,0},{-1,1}};
        for (auto& km : kingMoves) {
            int r = row + km[0], c = col + km[1];
            if (r >= 0 && r <= 7 && c >= 0 && c <= 7) {
                if ((team == 1 && state[r][c] <= 0) || (team == -1 && state[r][c] >= 0)) {
                    legalMoves.push_back(Move(row, col, r, c, 0));
                }
            }
        }
        
        // Castling
        if (piece == 6 && row == 7 && col == 4 && !whiteKingMoved) {
            if (!whiteRookHMoved && state[7][7] == 4 && state[7][5] == 0 && state[7][6] == 0) {
                legalMoves.push_back(Move(7, 4, 7, 6, 0));
            }
            if (!whiteRookAMoved && state[7][0] == 4 && state[7][1] == 0 && state[7][2] == 0 && state[7][3] == 0) {
                legalMoves.push_back(Move(7, 4, 7, 2, 0));
            }
        }
        if (piece == -6 && row == 0 && col == 4 && !blackKingMoved) {
            if (!blackRookHMoved && state[0][7] == -4 && state[0][5] == 0 && state[0][6] == 0) {
                legalMoves.push_back(Move(0, 4, 0, 6, 0));
            }
            if (!blackRookAMoved && state[0][0] == -4 && state[0][1] == 0 && state[0][2] == 0 && state[0][3] == 0) {
                legalMoves.push_back(Move(0, 4, 0, 2, 0));
            }
        }
    }

    // Filter moves that leave king in check
    auto savedState = state;
    auto savedFlags = std::make_tuple(whiteKingMoved, blackKingMoved, whiteRookAMoved, whiteRookHMoved, blackRookAMoved, blackRookHMoved);
    
    std::vector<Move> filteredMoves;
    for (const Move& move : legalMoves) {
        state = savedState;
        std::tie(whiteKingMoved, blackKingMoved, whiteRookAMoved, whiteRookHMoved, blackRookAMoved, blackRookHMoved) = savedFlags;
        
        movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol, move.promotion);
        
        bool wCheck, bCheck;
        int wkr, wkc, bkr, bkc;
        kingCheckCheck(wCheck, bCheck, wkr, wkc, bkr, bkc);
        
        if ((team == 1 && !wCheck) || (team == -1 && !bCheck)) {
            filteredMoves.push_back(move);
        }
    }
    
    state = savedState;
    std::tie(whiteKingMoved, blackKingMoved, whiteRookAMoved, whiteRookHMoved, blackRookAMoved, blackRookHMoved) = savedFlags;
    
    return filteredMoves;
}

std::vector<Move> Board::getAllLegalMoves(int team) {
    std::vector<Move> allMoves;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (state[r][c] * team > 0) {
                auto moves = getLegalMoves(r, c);
                allMoves.insert(allMoves.end(), moves.begin(), moves.end());
            }
        }
    }
    return allMoves;
}

int Board::checkForWin(int team) {
    auto allMoves = getAllLegalMoves(team);
    if (allMoves.empty()) {
        bool wCheck, bCheck;
        int wkr, wkc, bkr, bkc;
        kingCheckCheck(wCheck, bCheck, wkr, wkc, bkr, bkc);
        
        if (team == 1) {
            return wCheck ? -1 : 0; // -1 = white checkmate, 0 = stalemate
        } else {
            return bCheck ? 1 : 0; // 1 = black checkmate, 0 = stalemate
        }
    }
    return 2; // Game continues
}

bool Board::isInCheck(bool white) {
    bool wCheck, bCheck;
    int wkr, wkc, bkr, bkc;
    kingCheckCheck(wCheck, bCheck, wkr, wkc, bkr, bkc);
    return white ? wCheck : bCheck;
}
