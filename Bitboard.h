#pragma once
#include <cstdint>
#include <string>
typedef uint64_t Bitboard;
// Bitboard utilities
namespace BB {
    // Constants
    constexpr Bitboard EMPTY = 0ULL;
    constexpr Bitboard FULL = 0xFFFFFFFFFFFFFFFFULL;
    // Ranks
    constexpr Bitboard RANK_1 = 0x00000000000000FFULL;
    constexpr Bitboard RANK_2 = 0x000000000000FF00ULL;
    constexpr Bitboard RANK_3 = 0x0000000000FF0000ULL;
    constexpr Bitboard RANK_4 = 0x00000000FF000000ULL;
    constexpr Bitboard RANK_5 = 0x000000FF00000000ULL;
    constexpr Bitboard RANK_6 = 0x0000FF0000000000ULL;
    constexpr Bitboard RANK_7 = 0x00FF000000000000ULL;
    constexpr Bitboard RANK_8 = 0xFF00000000000000ULL;
    // Files
    constexpr Bitboard FILE_A = 0x0101010101010101ULL;
    constexpr Bitboard FILE_B = 0x0202020202020202ULL;
    constexpr Bitboard FILE_C = 0x0404040404040404ULL;
    constexpr Bitboard FILE_D = 0x0808080808080808ULL;
    constexpr Bitboard FILE_E = 0x1010101010101010ULL;
    constexpr Bitboard FILE_F = 0x2020202020202020ULL;
    constexpr Bitboard FILE_G = 0x4040404040404040ULL;
    constexpr Bitboard FILE_H = 0x8080808080808080ULL;
    // Bit manipulation
    inline int popCount(Bitboard bb) {
        #ifdef __GNUC__
        return __builtin_popcountll(bb);
        #else
        int count = 0;
        while (bb) {
            count++;
            bb &= bb - 1;
        }
        return count;
        #endif
    }
    inline int lsb(Bitboard bb) {
        #ifdef __GNUC__
        return __builtin_ctzll(bb);
        #else
        int pos = 0;
        if (!bb) return -1;
        while (!(bb & 1)) {
            bb >>= 1;
            pos++;
        }
        return pos;
        #endif
    }
    inline int msb(Bitboard bb) {
        #ifdef __GNUC__
        return 63 - __builtin_clzll(bb);
        #else
        int pos = 0;
        while (bb >>= 1) pos++;
        return pos;
        #endif
    }
    inline Bitboard popLsb(Bitboard& bb) {
        int sq = lsb(bb);
        bb &= bb - 1;
        return 1ULL << sq;
    }
    inline void setBit(Bitboard& bb, int square) {
        bb |= (1ULL << square);
    }
    inline void clearBit(Bitboard& bb, int square) {
        bb &= ~(1ULL << square);
    }
    inline bool testBit(Bitboard bb, int square) {
        return (bb & (1ULL << square)) != 0;
    }
    // Square to bitboard
    inline Bitboard squareBB(int square) {
        return 1ULL << square;
    }
    std::string toString(Bitboard bb);
}

class AttackTables {
public:
    static void init();
    static Bitboard pawnAttacks[2][64];      // [color][square]
    static Bitboard knightAttacks[64];
    static Bitboard kingAttacks[64];
    static Bitboard rookMasks[64];
    static Bitboard bishopMasks[64];
    static Bitboard rookAttacks[64][4096];
    static Bitboard bishopAttacks[64][512];
    static Bitboard rookMagics[64];
    static Bitboard bishopMagics[64];
    static int rookShifts[64];
    static int bishopShifts[64];
    static Bitboard getRookAttacks(int square, Bitboard occupied);
    static Bitboard getBishopAttacks(int square, Bitboard occupied);
    static Bitboard getQueenAttacks(int square, Bitboard occupied);
private:
    static void initPawnAttacks();
    static void initKnightAttacks();
    static void initKingAttacks();
    static void initSlidingAttacks();
    static Bitboard generateRookMask(int square);
    static Bitboard generateBishopMask(int square);
    static Bitboard generateRookAttacks(int square, Bitboard occupied);
    static Bitboard generateBishopAttacks(int square, Bitboard occupied);
};