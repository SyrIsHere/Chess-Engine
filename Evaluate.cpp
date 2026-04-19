#include "Evaluate.h"
#include <cmath>

double Evaluate::evaluate(Board& board) {
    double score = 0.0;

    for (int ri = 0; ri < 8; ri++) {
        for (int ci = 0; ci < 8; ci++) {
            int p = board.state[ri][ci];
            if (p == 0 || p == 7 || p == -7) continue;

            int team = (p > 0) ? 1 : -1;
            int absP = std::abs(p);

            // Material values
            if (absP == 1) { // Pawns
                score += 1.0 * team;
                
                // Doubled pawns penalty
                if (team == 1 && ri + 1 <= 7 && board.state[ri + 1][ci] == 1) {
                    score -= 0.5;
                } else if (team == -1 && ri - 1 >= 0 && board.state[ri - 1][ci] == -1) {
                    score += 0.5;
                }
            }
            else if (absP == 2) { // Knights
                score += 3.0 * team;
                
                // Position bonus
                if ((ri == 3 || ri == 4) && (ci == 3 || ci == 4)) score += 0.5 * team;
                else if ((ri == 2 || ri == 5) && (ci == 2 || ci == 5)) score += 0.25 * team;
                else if ((ri == 1 || ri == 6) && (ci == 1 || ci == 6)) score -= 0.25 * team;
                else if ((ri == 0 || ri == 7) && (ci == 0 || ci == 7)) score -= 0.5 * team;
            }
            else if (absP == 3) { // Bishops
                score += 3.0 * team;
                
                if ((ri == 3 || ri == 4) && (ci == 3 || ci == 4)) score += 0.5 * team;
                else if ((ri == 2 || ri == 5) && (ci == 2 || ci == 5)) score += 0.25 * team;
            }
            else if (absP == 4) { // Rooks
                score += 5.0 * team;
            }
            else if (absP == 5) { // Queens
                score += 9.0 * team;
            }
            else if (absP == 6) { // Kings
                if (team == 1) {
                    if ((ri == 7 && ci == 2) || (ri == 7 && ci == 6)) score += 0.75;
                    else if ((ri == 7 && ci == 1) || (ri == 7 && ci == 7)) score += 0.25;
                } else {
                    if ((ri == 0 && ci == 2) || (ri == 0 && ci == 6)) score -= 0.75;
                    else if ((ri == 0 && ci == 1) || (ri == 0 && ci == 7)) score -= 0.25;
                }
            }
        }
    }

    return score;
}
