#include "SearchAdvanced.h"
#include <algorithm>
#include <iostream>
#include <cstring>
const int INF = 30000;
const int MATE_VALUE = 29000;
SearchAdvanced::SearchAdvanced() : tt(64) {
    clearHistory();
    Zobrist::init();
}
SearchAdvanced::~SearchAdvanced() {}
void SearchAdvanced::clearHistory() {
    std::memset(killerMoves, 0, sizeof(killerMoves));
    std::memset(history, 0, sizeof(history));
}
void SearchAdvanced::stop() {
    info.stopped = true;
}
bool SearchAdvanced::shouldStop() {
    if (info.stopped) return true;
    if (info.nodes % 1024 == 0) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
        if (elapsed.count() >= limits.maxTime) {
            info.stopped = true;
            return true;
        }
    }
    if (info.nodes >= limits.maxNodes) {
        info.stopped = true;
        return true;
    }
    return false;
}
Move SearchAdvanced::findBestMove(Board& board, int team, SearchLimits searchLimits) {
    limits = searchLimits;
    info = SearchInfo();
    startTime = std::chrono::steady_clock::now();
    clearHistory();
    tt.newSearch();
    int score = iterativeDeepening(board, team);
    auto endTime = std::chrono::steady_clock::now();
    info.time = std::chrono::duration<double>(endTime - startTime).count();
    info.score = score;
    return info.bestMove;
}
int SearchAdvanced::iterativeDeepening(Board& board, int team) {
    Move bestMove;
    int bestScore = -INF;
    for (int depth = 1; depth <= limits.maxDepth; depth++) {
        if (shouldStop()) break;
        info.depth = depth;
        auto depthStart = std::chrono::steady_clock::now();
        int score = alphaBeta(board, team, depth, 0, -INF, INF, true);
        auto depthEnd = std::chrono::steady_clock::now();
        info.time = std::chrono::duration<double>(depthEnd - startTime).count();
        if (info.stopped) break;
        // Get best move from TT
        Move ttMove;
        int dummy;
        uint64_t key = Zobrist::hash(board);
        if (tt.probe(key, depth, -INF, INF, dummy, ttMove)) {
            bestMove = ttMove;
        }
        bestScore = score;
        info.bestMove = bestMove;
        info.score = score;
        // Print info (UCI-like) - visible in real-time
        std::cout << "info depth " << depth
                  << " score cp " << score
                  << " nodes " << info.nodes
                  << " time " << (int)(info.time * 1000)
                  << " nps " << (int)(info.nodes / (info.time + 0.001))
                  << std::endl;
        // Mate found
        if (abs(score) > MATE_VALUE - 100) break;
    }
    return bestScore;
}
int SearchAdvanced::alphaBeta(Board& board, int team, int depth, int ply,
                              int alpha, int beta, bool nullMove) {
    if (shouldStop()) return 0;
    
    info.nodes++;
    info.seldepth = std::max(info.seldepth, ply);
    
    // Check for draw by repetition or 50-move rule (simplified)
    
    // TT probe
    uint64_t key = Zobrist::hash(board);
    Move ttMove;
    int ttScore;
    if (tt.probe(key, depth, alpha, beta, ttScore, ttMove)) {
        if (ply > 0) return ttScore;
    }
    
    // Leaf node - quiescence search
    if (depth <= 0) {
        return quiescence(board, team, alpha, beta, ply);
    }
    // Null move pruning
    if (nullMove && depth >= 3 && canDoNullMove(board, team) && !board.isInCheck(team == 1)) {
        int R = 2;  // Reduction
        // Make null move (skip turn)
        int score = -alphaBeta(board, -team, depth - 1 - R, ply + 1, -beta, -beta + 1, false);
        if (score >= beta) {
            return beta;
        }
    }
    std::vector<Move> moves = board.getAllLegalMoves(team);
    // Checkmate or stalemate
    if (moves.empty()) {
        if (board.isInCheck(team == 1)) {
            return -MATE_VALUE + ply;  // Checkmate
        }
        return 0;  // Stalemate
    }
    // Move ordering
    orderMoves(moves, board, ttMove, ply);
    int bestScore = -INF;
    Move bestMove;
    NodeType nodeType = NodeType::UPPER;
    int legalMoves = 0;
    for (const Move& move : moves) {
        Board childBoard = board;
        childBoard.movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol, move.promotion);
        legalMoves++;
        int score;
        // PVS (Principal Variation Search)
        if (legalMoves == 1) {
            score = -alphaBeta(childBoard, -team, depth - 1, ply + 1, -beta, -alpha, true);
        } else {
            // Late move reduction
            int reduction = 0;
            if (legalMoves > 4 && depth >= 3 && !isCapture(board, move)) {
                reduction = 1;
            }
            // Null window search
            score = -alphaBeta(childBoard, -team, depth - 1 - reduction, ply + 1, -alpha - 1, -alpha, true);
            // Re-search if necessary
            if (score > alpha && score < beta) {
                score = -alphaBeta(childBoard, -team, depth - 1, ply + 1, -beta, -alpha, true);
            }
        }
        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
            if (score > alpha) {
                alpha = score;
                nodeType = NodeType::EXACT;
                if (score >= beta) {
                    // Beta cutoff
                    nodeType = NodeType::LOWER;
                    // Update killers and history
                    if (!isCapture(board, move)) {
                        updateKillers(move, ply);
                        updateHistory(move, team, depth);
                    }
                    break;
                }
            }
        }
    }
    // Store in TT
    int eval = evaluator.evaluate(board);
    tt.store(key, depth, bestScore, eval, nodeType, bestMove);
    return bestScore;
}
int SearchAdvanced::quiescence(Board& board, int team, int alpha, int beta, int ply) {
    if (shouldStop()) return 0;
    info.nodes++;
    int standPat = evaluator.evaluate(board);
    if (team == -1) standPat = -standPat;
    if (standPat >= beta) return beta;
    if (alpha < standPat) alpha = standPat;
    std::vector<Move> moves = board.getAllLegalMoves(team);
    // Only search captures and checks
    std::vector<Move> captureMoves;
    for (const Move& move : moves) {
        if (isCapture(board, move)) {
            captureMoves.push_back(move);
        }
    }
    // Order captures by MVV-LVA
    std::sort(captureMoves.begin(), captureMoves.end(),
        [this, &board](const Move& a, const Move& b) {
            return mvvLva(board, a) > mvvLva(board, b);
        });
    for (const Move& move : captureMoves) {
        Board childBoard = board;
        childBoard.movePiece(move.fromRow, move.fromCol, move.toRow, move.toCol, move.promotion);
        int score = -quiescence(childBoard, -team, -beta, -alpha, ply + 1);
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}
void SearchAdvanced::orderMoves(std::vector<Move>& moves, Board& board, Move ttMove, int ply) {
    std::sort(moves.begin(), moves.end(),
        [this, &board, &ttMove, ply](const Move& a, const Move& b) {
            return getMoveScore(a, board, ttMove, ply) > getMoveScore(b, board, ttMove, ply);
        });
}
int SearchAdvanced::getMoveScore(const Move& move, Board& board, Move ttMove, int ply) {
    int score = 0;
    
    // TT move
    if (move.fromRow == ttMove.fromRow && move.fromCol == ttMove.fromCol &&
        move.toRow == ttMove.toRow && move.toCol == ttMove.toCol) {
        return 10000000;
    }
    
    // Captures (MVV-LVA)
    if (isCapture(board, move)) {
        score += 1000000 + mvvLva(board, move);
    }
    
    // Promotions
    if (move.promotion != 0) {
        score += 900000 + abs(move.promotion) * 100;
    }
    // Killer moves
    for (int i = 0; i < 2; i++) {
        if (move.fromRow == killerMoves[ply][i].fromRow &&
            move.fromCol == killerMoves[ply][i].fromCol &&
            move.toRow == killerMoves[ply][i].toRow &&
            move.toCol == killerMoves[ply][i].toCol) {
            score += 800000 - i * 100000;
        }
    }
    // History heuristic
    int team = (board.state[move.fromRow][move.fromCol] > 0) ? 0 : 1;
    int from = move.fromRow * 8 + move.fromCol;
    int to = move.toRow * 8 + move.toCol;
    score += history[team][from][to];
    return score;
}
bool SearchAdvanced::isCapture(Board& board, const Move& move) {
    int target = board.state[move.toRow][move.toCol];
    return target != 0 && target != 7 && target != -7;
}
int SearchAdvanced::mvvLva(Board& board, const Move& move) {
    int victim = abs(board.state[move.toRow][move.toCol]);
    int attacker = abs(board.state[move.fromRow][move.fromCol]);
    // MVV-LVA: Most Valuable Victim - Least Valuable Attacker
    return victim * 10 - attacker;
}
bool SearchAdvanced::canDoNullMove(Board& board, int team) {
    // Don't do null move in endgame or when in check
    int pieceCount = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int piece = board.state[r][c];
            if (piece != 0 && piece != 7 && piece != -7) {
                if ((team == 1 && piece > 0) || (team == -1 && piece < 0)) {
                    pieceCount++;
                }
            }
        }
    }
    return pieceCount > 3;
}
void SearchAdvanced::updateKillers(Move move, int ply) {
    if (ply >= 64) return;
    // Shift killers
    if (!(move.fromRow == killerMoves[ply][0].fromRow &&
          move.fromCol == killerMoves[ply][0].fromCol &&
          move.toRow == killerMoves[ply][0].toRow &&
          move.toCol == killerMoves[ply][0].toCol)) {
        killerMoves[ply][1] = killerMoves[ply][0];
        killerMoves[ply][0] = move;
    }
}
void SearchAdvanced::updateHistory(Move move, int team, int depth) {
    int colorIdx = (team == 1) ? 0 : 1;
    int from = move.fromRow * 8 + move.fromCol;
    int to = move.toRow * 8 + move.toCol;
    history[colorIdx][from][to] += depth * depth;
    // Cap history values
    if (history[colorIdx][from][to] > 10000) {
        history[colorIdx][from][to] = 10000;
    }
}