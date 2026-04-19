#pragma once
#include <cstdint>
#include "Board.h"
enum class NodeType : uint8_t {
    EXACT,      // PV node
    LOWER,      // Beta cutoff (fail-high)
    UPPER       // Alpha cutoff (fail-low)
};
struct TTEntry {
    uint64_t key;
    int16_t score;
    int16_t eval;
    Move bestMove;
    uint8_t depth;
    NodeType type;
    uint8_t age;
    TTEntry() : key(0), score(0), eval(0), depth(0),
                type(NodeType::EXACT), age(0) {}
};
class TranspositionTable {
public:
    TranspositionTable(size_t sizeMB = 64);
    ~TranspositionTable();
    void clear();
    void resize(size_t sizeMB);
    void newSearch();
    bool probe(uint64_t key, int depth, int alpha, int beta,
               int& score, Move& bestMove);
    void store(uint64_t key, int depth, int score, int eval,
               NodeType type, Move bestMove);
    int getHashFull() const;
    size_t getSize() const { return size; }
private:
    TTEntry* table;
    size_t size;
    uint8_t currentAge;
    size_t index(uint64_t key) const {
        return key % size;
    }
};
// Zobrist hashing for position keys
class Zobrist {
public:
    static void init();
    static uint64_t hash(const Board& board);
    
    static uint64_t pieceKeys[12][64];      // [piece][square]
    static uint64_t castleKeys[16];         // castling rights
    static uint64_t enPassantKeys[8];       // en passant file
    static uint64_t sideKey;                // side to move
    
private:
    static uint64_t random64();
};