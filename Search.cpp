#include "Search.h"
#include <algorithm>
#include <iostream>

Search::Search() {}

Board Search::copyBoard(Board& board) {
    Board newBoard;
    newBoard.state = board.state;
    newBoard.whiteKingMoved = board.whiteKingMoved;
    newBoard.blackKingMoved = board.blackKingMoved;
    newBoard.whiteRookAMoved = board.whiteRookAMoved;
    newBoard.whiteRookHMoved = board.whiteRookHMoved;
    newBoard.blackRookAMoved = board.blackRookAMoved;
    newBoard.blackRookHMoved = board.blackRookHMoved;
    return newBoard;
}

int Search::moveOrderScore(Board& board, const Move& move) {
    int targetPiece = board.state[move.toRow][move.toCol];
    int score = 0;

    if (move.promotion != 0) score += 100;
    if (targetPiece != 0 && targetPiece != 7 && targetPiece != -7) {
        score += std::abs(targetPiece) * 10;
    }

    return score;
}

std::pair<double, Move> Search::minmax(Board& board, int team, int depth, double alpha, double beta) {
    std::vector<Move> legalMoves = board.getAllLegalMoves(team);

    if (legalMoves.empty()) {
        bool whiteInCheck, blackInCheck;
        int wkr, wkc, bkr, bkc;
        board.kingCheckCheck(whiteInCheck, blackInCheck, wkr, wkc, bkr, bkc);

        if (team == 1) {
            return {whiteInCheck ? -1000.0 - depth : 0.0, Move()};
        } else {
            return {blackInCheck ? 1000.0 + depth : 0.0, Move()};
        }
    }

    if (depth == 0) {
        return {evaluator.evaluate(board), Move()};
    }

    std::sort(legalMoves.begin(), legalMoves.end(), 
        [this, &board](const Move& a, const Move& b) {
            return moveOrderScore(board, a) > moveOrderScore(board, b);
        });

    double bestEval = (team == 1) ? -99999.0 : 99999.0;
    Move bestMove;

    for (const Move& m : legalMoves) {
        Board childBoard = copyBoard(board);
        childBoard.movePiece(m.fromRow, m.fromCol, m.toRow, m.toCol, m.promotion);

        auto [eval, _] = minmax(childBoard, team * -1, depth - 1, alpha, beta);

        if (team == 1) {
            if (eval > bestEval) {
                bestEval = eval;
                bestMove = m;
            }
            alpha = std::max(alpha, bestEval);
        } else {
            if (eval < bestEval) {
                bestEval = eval;
                bestMove = m;
            }
            beta = std::min(beta, bestEval);
        }

        if (beta <= alpha) break;
    }

    return {bestEval, bestMove};
}
