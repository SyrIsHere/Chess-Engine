#include "Bitboard.h"
#include <sstream>
Bitboard AttackTables::pawnAttacks[2][64];
Bitboard AttackTables::knightAttacks[64];
Bitboard AttackTables::kingAttacks[64];
Bitboard AttackTables::rookMasks[64];
Bitboard AttackTables::bishopMasks[64];
Bitboard AttackTables::rookAttacks[64][4096];
Bitboard AttackTables::bishopAttacks[64][512];
Bitboard AttackTables::rookMagics[64];
Bitboard AttackTables::bishopMagics[64];
int AttackTables::rookShifts[64];
int AttackTables::bishopShifts[64];
std::string BB::toString(Bitboard bb) {
    std::stringstream ss;
    for (int rank = 7; rank >= 0; rank--) {
        ss << (rank + 1) << " ";
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            ss << (testBit(bb, square) ? "1 " : ". ");
        }
        ss << "\n";
    }
    ss << "  a b c d e f g h\n";
    return ss.str();
}
void AttackTables::init() {
    initPawnAttacks();
    initKnightAttacks();
    initKingAttacks();
    initSlidingAttacks();
}
void AttackTables::initPawnAttacks() {
    for (int sq = 0; sq < 64; sq++) {
        int rank = sq / 8;
        int file = sq % 8;
        
        // White pawns
        Bitboard attacks = 0;
        if (rank < 7) {
            if (file > 0) BB::setBit(attacks, sq + 7);
            if (file < 7) BB::setBit(attacks, sq + 9);
        }
        pawnAttacks[0][sq] = attacks;
        
        // Black pawns
        attacks = 0;
        if (rank > 0) {
            if (file > 0) BB::setBit(attacks, sq - 9);
            if (file < 7) BB::setBit(attacks, sq - 7);
        }
        pawnAttacks[1][sq] = attacks;
    }
}
void AttackTables::initKnightAttacks() {
    int knightMoves[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for (int sq = 0; sq < 64; sq++) {
        int rank = sq / 8;
        int file = sq % 8;
        Bitboard attacks = 0;
        for (auto& move : knightMoves) {
            int newRank = rank + move[0];
            int newFile = file + move[1];
            if (newRank >= 0 && newRank < 8 && newFile >= 0 && newFile < 8) {
                BB::setBit(attacks, newRank * 8 + newFile);
            }
        }
        knightAttacks[sq] = attacks;
    }
}
void AttackTables::initKingAttacks() {
    int kingMoves[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
    for (int sq = 0; sq < 64; sq++) {
        int rank = sq / 8;
        int file = sq % 8;
        Bitboard attacks = 0;
        for (auto& move : kingMoves) {
            int newRank = rank + move[0];
            int newFile = file + move[1];
            if (newRank >= 0 && newRank < 8 && newFile >= 0 && newFile < 8) {
                BB::setBit(attacks, newRank * 8 + newFile);
            }
        }
        kingAttacks[sq] = attacks;
    }
}
Bitboard AttackTables::generateRookMask(int square) {
    Bitboard mask = 0;
    int rank = square / 8;
    int file = square % 8;
    for (int r = rank + 1; r < 7; r++) BB::setBit(mask, r * 8 + file);
    for (int r = rank - 1; r > 0; r--) BB::setBit(mask, r * 8 + file);
    for (int f = file + 1; f < 7; f++) BB::setBit(mask, rank * 8 + f);
    for (int f = file - 1; f > 0; f--) BB::setBit(mask, rank * 8 + f);
    return mask;
}
Bitboard AttackTables::generateBishopMask(int square) {
    Bitboard mask = 0;
    int rank = square / 8;
    int file = square % 8;
    for (int r = rank + 1, f = file + 1; r < 7 && f < 7; r++, f++)
        BB::setBit(mask, r * 8 + f);
    for (int r = rank + 1, f = file - 1; r < 7 && f > 0; r++, f--)
        BB::setBit(mask, r * 8 + f);
    for (int r = rank - 1, f = file + 1; r > 0 && f < 7; r--, f++)
        BB::setBit(mask, r * 8 + f);
    for (int r = rank - 1, f = file - 1; r > 0 && f > 0; r--, f--)
        BB::setBit(mask, r * 8 + f);
    return mask;
}
Bitboard AttackTables::generateRookAttacks(int square, Bitboard occupied) {
    Bitboard attacks = 0;
    int rank = square / 8;
    int file = square % 8;
    for (int r = rank + 1; r < 8; r++) {
        BB::setBit(attacks, r * 8 + file);
        if (BB::testBit(occupied, r * 8 + file)) break;
    }
    for (int r = rank - 1; r >= 0; r--) {
        BB::setBit(attacks, r * 8 + file);
        if (BB::testBit(occupied, r * 8 + file)) break;
    }
    for (int f = file + 1; f < 8; f++) {
        BB::setBit(attacks, rank * 8 + f);
        if (BB::testBit(occupied, rank * 8 + f)) break;
    }
    for (int f = file - 1; f >= 0; f--) {
        BB::setBit(attacks, rank * 8 + f);
        if (BB::testBit(occupied, rank * 8 + f)) break;
    }
    return attacks;
}
Bitboard AttackTables::generateBishopAttacks(int square, Bitboard occupied) {
    Bitboard attacks = 0;
    int rank = square / 8;
    int file = square % 8;
    for (int r = rank + 1, f = file + 1; r < 8 && f < 8; r++, f++) {
        BB::setBit(attacks, r * 8 + f);
        if (BB::testBit(occupied, r * 8 + f)) break;
    }
    for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; r++, f--) {
        BB::setBit(attacks, r * 8 + f);
        if (BB::testBit(occupied, r * 8 + f)) break;
    }
    for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; r--, f++) {
        BB::setBit(attacks, r * 8 + f);
        if (BB::testBit(occupied, r * 8 + f)) break;
    }
    for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
        BB::setBit(attacks, r * 8 + f);
        if (BB::testBit(occupied, r * 8 + f)) break;
    }
    return attacks;
}
void AttackTables::initSlidingAttacks() {
    for (int sq = 0; sq < 64; sq++) {
        rookMasks[sq] = generateRookMask(sq);
        bishopMasks[sq] = generateBishopMask(sq);
        rookShifts[sq] = 64 - BB::popCount(rookMasks[sq]);
        bishopShifts[sq] = 64 - BB::popCount(bishopMasks[sq]);
        int rookBits = BB::popCount(rookMasks[sq]);
        int bishopBits = BB::popCount(bishopMasks[sq]);
        for (int i = 0; i < (1 << rookBits); i++) {
            Bitboard occupied = 0;
            Bitboard mask = rookMasks[sq];
            int count = 0;
            while (mask) {
                int bit = BB::lsb(mask);
                if (i & (1 << count)) BB::setBit(occupied, bit);
                BB::clearBit(mask, bit);
                count++;
            }
            rookAttacks[sq][i] = generateRookAttacks(sq, occupied);
        }
        for (int i = 0; i < (1 << bishopBits); i++) {
            Bitboard occupied = 0;
            Bitboard mask = bishopMasks[sq];
            int count = 0;
            while (mask) {
                int bit = BB::lsb(mask);
                if (i & (1 << count)) BB::setBit(occupied, bit);
                BB::clearBit(mask, bit);
                count++;
            }
            bishopAttacks[sq][i] = generateBishopAttacks(sq, occupied);
        }
    }
}
Bitboard AttackTables::getRookAttacks(int square, Bitboard occupied) {
    occupied &= rookMasks[square];
    int index = (occupied * 0x0080201002040810ULL) >> rookShifts[square];
    return rookAttacks[square][index & 0xFFF];
}
Bitboard AttackTables::getBishopAttacks(int square, Bitboard occupied) {
    occupied &= bishopMasks[square];
    int index = (occupied * 0x0202020202020202ULL) >> bishopShifts[square];
    return bishopAttacks[square][index & 0x1FF];
}
Bitboard AttackTables::getQueenAttacks(int square, Bitboard occupied) {
    return getRookAttacks(square, occupied) | getBishopAttacks(square, occupied);
}