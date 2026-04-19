#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <map>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include "Board.h"
#include "SearchAdvanced.h"
class Game {
public:
    Game();
    ~Game();
    void run();
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    Board board;
    SearchAdvanced search;
    bool running;
    int screenSize;
    int squareSize;
    int pieceSize;
    int boardOffset;
    int selectedRow, selectedCol;
    int hoverRow, hoverCol;
    bool pieceSelected;
    int activeTeam;
    int whiteKingCheckRow, whiteKingCheckCol;
    int blackKingCheckRow, blackKingCheckCol;
    bool promotionPossible;
    std::vector<Move> matchedMoves;
    bool whiteWin, blackWin, stalemate;
    std::map<int, SDL_Texture*> pieceTextures;
    TTF_Font* font;
    TTF_Font* fontLarge;
    TTF_Font* fontCoords;
    // Search settings
    SearchLimits searchLimits;
    std::atomic<bool> thinking;
    std::thread searchThread;
    std::mutex boardMutex;
    Move pendingMove;
    std::atomic<bool> hasPendingMove;
    // Search info for display
    std::mutex infoMutex;
    SearchInfo currentSearchInfo;
    bool init();
    void loadPieceImages();
    void handleEvents();
    void update();
    void draw();
    void cleanup();
    void startAIThinking();
    void aiThinkingThread();
    void applyPendingMove();
    void drawBoard();
    void drawCoordinates();
    void drawPieces();
    void drawLegalMoveIndicators();
    void drawPromotionDialog();
    void drawGameResult();
    void drawSearchInfo();
    void drawShadow(int x, int y, int size);
    void drawText(const std::string& text, int x, int y, SDL_Color color, TTF_Font* useFont = nullptr);
};