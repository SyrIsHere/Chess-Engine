#pragma once
#include <vector>
#include <array>
struct Move {
    int fromRow, fromCol;
    int toRow, toCol;
    int promotion;
    Move() : fromRow(0), fromCol(0), toRow(0), toCol(0), promotion(0) {}
    Move(int fr, int fc, int tr, int tc, int p = 0)
        : fromRow(fr), fromCol(fc), toRow(tr), toCol(tc), promotion(p) {}
};
class Board {
public:
    std::array<std::array<int, 8>, 8> state;
    bool whiteKingMoved;
    bool blackKingMoved;
    bool whiteRookAMoved;
    bool whiteRookHMoved;
    bool blackRookAMoved;
    bool blackRookHMoved;
    Board();
    void movePiece(int row, int col, int newRow, int newCol, int promotionPiece);
    std::vector<Move> getLegalMoves(int row, int col);
    std::vector<Move> getAllLegalMoves(int team);
    int checkForWin(int team);
    bool isInCheck(bool white);
    void kingCheckCheck(bool& whiteInCheck, bool& blackInCheck,
                       int& whiteKingRow, int& whiteKingCol,
                       int& blackKingRow, int& blackKingCol);
private:
    std::vector<Move> longRangeRecursion(int row, int col, int dirX, int dirY, int team);
};