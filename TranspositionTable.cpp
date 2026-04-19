#include "TranspositionTable.h"
#include <cstring>
#include <random>
// Zobrist keys
uint64_t Zobrist::pieceKeys[12][64];
uint64_t Zobrist::castleKeys[16];
uint64_t Zobrist::enPassantKeys[8];
uint64_t Zobrist::sideKey;
uint64_t Zobrist::random64() {
    static std::mt19937_64 gen(0x1234567890ABCDEFULL);
    static std::uniform_int_distribution<uint64_t> dist;
    return dist(gen);
}
void Zobrist::init() {
    for (int piece = 0; piece < 12; piece++) {
        for (int sq = 0; sq < 64; sq++) {
            pieceKeys[piece][sq] = random64();
        }
    }
    for (int i = 0; i < 16; i++) {
        castleKeys[i] = random64();
    }
    for (int i = 0; i < 8; i++) {
        enPassantKeys[i] = random64();
    }
    sideKey = random64();
}
uint64_t Zobrist::hash(const Board& board) {
    uint64_t key = 0;
    // Hash pieces
    for (int sq = 0; sq < 64; sq++) {
        int piece = board.state[sq / 8][sq % 8];
        if (piece != 0 && piece != 7 && piece != -7) {
            int pieceIndex = (piece > 0) ? (piece - 1) : (6 + (-piece - 1));
            key ^= pieceKeys[pieceIndex][sq];
        }
    }
    // Hash castling rights
    int castleRights = 0;
    if (board.whiteKingMoved == false) {
        if (board.whiteRookHMoved == false) castleRights |= 1;
        if (board.whiteRookAMoved == false) castleRights |= 2;
    }
    if (board.blackKingMoved == false) {
        if (board.blackRookHMoved == false) castleRights |= 4;
        if (board.blackRookAMoved == false) castleRights |= 8;
    }
    key ^= castleKeys[castleRights];
    return key;
}
TranspositionTable::TranspositionTable(size_t sizeMB)
    : table(nullptr), size(0), currentAge(0) {
    resize(sizeMB);
}
TranspositionTable::~TranspositionTable() {
    delete[] table;
}
void TranspositionTable::clear() {
    if (table) {
        std::memset(table, 0, size * sizeof(TTEntry));
    }
    currentAge = 0;
}
void TranspositionTable::resize(size_t sizeMB) {
    delete[] table;
    size = (sizeMB * 1024 * 1024) / sizeof(TTEntry);
    table = new TTEntry[size];
    clear();
}
void TranspositionTable::newSearch() {
    currentAge++;
}
bool TranspositionTable::probe(uint64_t key, int depth, int alpha, int beta,
                               int& score, Move& bestMove) {
    TTEntry& entry = table[index(key)];
    if (entry.key != key) {
        return false;
    }
    bestMove = entry.bestMove;
    if (entry.depth >= depth) {
        score = entry.score;
        switch (entry.type) {
            case NodeType::EXACT:
                return true;
            case NodeType::LOWER:
                if (score >= beta) return true;
                break;
            case NodeType::UPPER:
                if (score <= alpha) return true;
                break;
        }
    }
    return false;
}
void TranspositionTable::store(uint64_t key, int depth, int score, int eval,
                               NodeType type, Move bestMove) {
    TTEntry& entry = table[index(key)];
    // Replace if: empty, same position, deeper search, or older age
    if (entry.key == 0 || entry.key == key ||
        depth >= entry.depth || entry.age < currentAge) {
        entry.key = key;
        entry.score = score;
        entry.eval = eval;
        entry.depth = depth;
        entry.type = type;
        entry.bestMove = bestMove;
        entry.age = currentAge;
    }
}
int TranspositionTable::getHashFull() const {
    int count = 0;
    int samples = std::min(size, (size_t)1000);
    for (int i = 0; i < samples; i++) {
        if (table[i].key != 0 && table[i].age == currentAge) {
            count++;
        }
    }
    return (count * 1000) / samples;
}