#include "Game.h"
#include "Bitboard.h"
#include <iostream>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
Game::Game() : window(nullptr), renderer(nullptr), running(true),
               screenSize(1000), squareSize(100), pieceSize(85), boardOffset(50),
               selectedRow(-1), selectedCol(-1), hoverRow(-1), hoverCol(-1),
               pieceSelected(false), activeTeam(1),
               whiteKingCheckRow(-1), whiteKingCheckCol(-1),
               blackKingCheckRow(-1), blackKingCheckCol(-1),
               promotionPossible(false), whiteWin(false), blackWin(false), stalemate(false),
               font(nullptr), fontLarge(nullptr), fontCoords(nullptr),
               thinking(false), hasPendingMove(false) {
    // Initialize search limits
    searchLimits.maxDepth = 8;
    searchLimits.maxTime = 3000;
    searchLimits.maxNodes = 1000000;
    AttackTables::init();
    Zobrist::init();
}
Game::~Game() {
    if (searchThread.joinable()) {
        search.stop();
        searchThread.join();
    }
    cleanup();
}
bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    if (TTF_Init() < 0) {
        std::cerr << "SDL_ttf init failed: " << TTF_GetError() << std::endl;
        return false;
    }
    window = SDL_CreateWindow("Chess Engine - C++ Edition",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              screenSize, screenSize, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image init failed: " << IMG_GetError() << std::endl;
        return false;
    }

    font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 24);
    if (!font) {
        font = TTF_OpenFont("arial.ttf", 24);
    }
    fontLarge = TTF_OpenFont("C:\\Windows\\Fonts\\arialbd.ttf", 36);
    if (!fontLarge) {
        fontLarge = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 36);
    }
    fontCoords = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 18);
    if (!fontCoords) {
        fontCoords = TTF_OpenFont("arial.ttf", 18);
    }
    loadPieceImages();
    return true;
}
void Game::loadPieceImages() {
    std::map<int, std::string> pieceFiles = {
        {1, "assets/wp.png"},
        {2, "assets/wn.png"},
        {3, "assets/wb.png"},
        {4, "assets/wr.png"},
        {5, "assets/wq.png"},
        {6, "assets/wk.png"},
        {-1, "assets/bp.png"},
        {-2, "assets/bn.png"},
        {-3, "assets/bb.png"},
        {-4, "assets/br.png"},
        {-5, "assets/bq.png"},
        {-6, "assets/bk.png"}
    };
    for (auto& [piece, filename] : pieceFiles) {
        SDL_Surface* surface = IMG_Load(filename.c_str());
        if (!surface) {
            std::cerr << "Failed to load " << filename << ": " << IMG_GetError() << std::endl;
            continue;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (texture) {
            pieceTextures[piece] = texture;
        }
    }
}
void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        if (event.type == SDL_MOUSEMOTION) {
            int mouseX = event.motion.x;
            int mouseY = event.motion.y;
            int boardX = mouseX - boardOffset;
            int boardY = mouseY - boardOffset;
            if (boardX >= 0 && boardX < 8 * squareSize && boardY >= 0 && boardY < 8 * squareSize) {
                hoverCol = boardX / squareSize;
                hoverRow = boardY / squareSize;
            } else {
                hoverRow = hoverCol = -1;
            }
        }
        if (activeTeam == 1 && event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            int mouseX = event.button.x;
            int mouseY = event.button.y;
            if (promotionPossible) {
                // Handle promotion selection
                int dialogY = screenSize / 2 - 120;
                if (mouseY >= dialogY + 60 && mouseY <= dialogY + 60 + pieceSize) {
                    int pieces[] = {2, 3, 4, 5};
                    for (int i = 0; i < 4; i++) {
                        int x = screenSize / 4 + 60 + i * 110;
                        if (mouseX >= x && mouseX <= x + pieceSize) {
                            for (const Move& move : matchedMoves) {
                                if (pieces[i] == move.promotion) {
                                    board.movePiece(selectedRow, selectedCol, move.toRow, move.toCol, move.promotion);
                                    selectedRow = selectedCol = -1;
                                    pieceSelected = false;
                                    activeTeam *= -1;
                                    int result = board.checkForWin(activeTeam);
                                    if (result == 1) whiteWin = true;
                                    else if (result == -1) blackWin = true;
                                    else if (result == 0) stalemate = true;
                                    promotionPossible = false;
                                    matchedMoves.clear();
                                    return;
                                }
                            }
                        }
                    }
                }
                return;
            }
            int boardX = mouseX - boardOffset;
            int boardY = mouseY - boardOffset;
            if (boardX >= 0 && boardX < 8 * squareSize && boardY >= 0 && boardY < 8 * squareSize) {
                int col = boardX / squareSize;
                int row = boardY / squareSize;
                int piece = board.state[row][col];
                if (pieceSelected) {
                    std::vector<Move> moves = board.getLegalMoves(selectedRow, selectedCol);
                    bool legalMove = false;
                    for (const Move& move : moves) {
                        if (row == move.toRow && col == move.toCol) {
                            legalMove = true;
                            break;
                        }
                    }
                    if (legalMove) {
                        matchedMoves.clear();
                        for (const Move& move : moves) {
                            if (row == move.toRow && col == move.toCol) {
                                matchedMoves.push_back(move);
                            }
                        }
                        if (matchedMoves.size() > 1) {
                            promotionPossible = true;
                            return;
                        } else {
                            Move move = matchedMoves[0];
                            board.movePiece(selectedRow, selectedCol, move.toRow, move.toCol, move.promotion);
                            selectedRow = selectedCol = -1;
                            pieceSelected = false;
                            activeTeam *= -1;
                            int result = board.checkForWin(activeTeam);
                            if (result == 1) whiteWin = true;
                            else if (result == -1) blackWin = true;
                            else if (result == 0) stalemate = true;
                            bool wCheck, bCheck;
                            board.kingCheckCheck(wCheck, bCheck, whiteKingCheckRow, whiteKingCheckCol,
                                               blackKingCheckRow, blackKingCheckCol);
                            return;
                        }
                    }
                    if (piece != 0) {
                        int clickedTeam = (piece > 0) ? 1 : -1;
                        if (clickedTeam == activeTeam) {
                            selectedRow = row;
                            selectedCol = col;
                            pieceSelected = true;
                        } else {
                            selectedRow = selectedCol = -1;
                            pieceSelected = false;
                        }
                    } else {
                        selectedRow = selectedCol = -1;
                        pieceSelected = false;
                    }
                } else {
                    if (piece != 0) {
                        int pieceTeam = (piece > 0) ? 1 : -1;
                        if (pieceTeam == activeTeam) {
                            selectedRow = row;
                            selectedCol = col;
                            pieceSelected = true;
                        }
                    }
                }
            }
        }
    }
}
void Game::update() {
    // Apply pending move from AI thread
    if (hasPendingMove.load()) {
        applyPendingMove();
    }
    // Opponent
    if (activeTeam == -1 && !whiteWin && !blackWin && !stalemate && !thinking.load()) {
        startAIThinking();
    }
}
void Game::startAIThinking() {
    if (searchThread.joinable()) {
        searchThread.join();
    }
    thinking.store(true);
    searchThread = std::thread(&Game::aiThinkingThread, this);
}
void Game::aiThinkingThread() {
    std::cout << "\n=== Engine Thinking ===" << std::endl;
    // Thread copy
    Board boardCopy;
    {
        std::lock_guard<std::mutex> lock(boardMutex);
        boardCopy = board;
    }
    // Search
    Move bestMove = search.findBestMove(boardCopy, -1, searchLimits);
    SearchInfo info = search.getInfo();
    {
        std::lock_guard<std::mutex> lock(infoMutex);
        currentSearchInfo = info;
    }
    std::cout << "Best move: " << (char)('a' + bestMove.fromCol) << (bestMove.fromRow + 1)
              << (char)('a' + bestMove.toCol) << (bestMove.toRow + 1) << std::endl;
    std::cout << "Score: " << info.score << " cp" << std::endl;
    std::cout << "Depth: " << info.depth << std::endl;
    std::cout << "Nodes: " << info.nodes << std::endl;
    std::cout << "Time: " << info.time << " s" << std::endl;
    std::cout << "NPS: " << (int)(info.nodes / (info.time + 0.001)) << std::endl;
    // Store pending move
    if (bestMove.fromRow >= 0) {
        pendingMove = bestMove;
        hasPendingMove.store(true);
    }
    thinking.store(false);
}
void Game::applyPendingMove() {
    std::lock_guard<std::mutex> lock(boardMutex);
    board.movePiece(pendingMove.fromRow, pendingMove.fromCol,
                    pendingMove.toRow, pendingMove.toCol, pendingMove.promotion);
    activeTeam = 1;
    int result = board.checkForWin(activeTeam);
    if (result == 1) whiteWin = true;
    else if (result == -1) blackWin = true;
    else if (result == 0) stalemate = true;
    bool wCheck, bCheck;
    board.kingCheckCheck(wCheck, bCheck, whiteKingCheckRow, whiteKingCheckCol,
                       blackKingCheckRow, blackKingCheckCol);
    hasPendingMove.store(false);
}
void Game::draw() {
    // Background gradient
    for (int y = 0; y < screenSize; y++) {
        int gray = 20 + (y * 15 / screenSize);
        SDL_SetRenderDrawColor(renderer, gray, gray, gray + 5, 255);
        SDL_RenderDrawLine(renderer, 0, y, screenSize, y);
    }
    drawBoard();
    drawCoordinates();
    drawLegalMoveIndicators();
    drawPieces();
    if (promotionPossible) {
        drawPromotionDialog();
    }
    if (whiteWin || blackWin || stalemate) {
        drawGameResult();
    }
    if (thinking) {
        drawSearchInfo();
    }
    SDL_RenderPresent(renderer);
}
void Game::drawSearchInfo() {
    if (!thinking.load()) return;
    SearchInfo info;
    {
        std::lock_guard<std::mutex> lock(infoMutex);
        info = currentSearchInfo;
    }
    // Semi-transparent background
    SDL_Rect bgRect = {10, screenSize - 120, 300, 110};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_RenderFillRect(renderer, &bgRect);
    SDL_SetRenderDrawColor(renderer, 100, 150, 255, 255);
    SDL_RenderDrawRect(renderer, &bgRect);
    SDL_Color textColor = {255, 255, 255, 255};
    std::string depthStr = "Depth: " + std::to_string(info.depth);
    std::string nodesStr = "Nodes: " + std::to_string(info.nodes);
    std::string timeStr = "Time: " + std::to_string((int)(info.time * 1000)) + " ms";
    int nps = (int)(info.nodes / (info.time + 0.001));
    std::string npsStr = "NPS: " + std::to_string(nps / 1000) + "K";
    drawText(depthStr, 20, screenSize - 110, textColor, font);
    drawText(nodesStr, 20, screenSize - 85, textColor, font);
    drawText(timeStr, 20, screenSize - 60, textColor, font);
    drawText(npsStr, 20, screenSize - 35, textColor, font);
}
void Game::drawShadow(int x, int y, int size) {
    for (int i = 0; i < 4; i++) {
        SDL_Rect shadowRect = {x + i + 2, y + i + 2, size - i * 2, size - i * 2};
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 30 - i * 5);
        SDL_RenderFillRect(renderer, &shadowRect);
    }
}
void Game::drawText(const std::string& text, int x, int y, SDL_Color color, TTF_Font* useFont) {
    if (!useFont) useFont = font;
    if (!useFont) return;
    SDL_Surface* surface = TTF_RenderText_Blended(useFont, text.c_str(), color);
    if (!surface) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect destRect = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &destRect);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}
void Game::drawBoard() {
    // Board shadow
    SDL_Rect boardShadow = {boardOffset + 5, boardOffset + 5, 8 * squareSize, 8 * squareSize};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 80);
    SDL_RenderFillRect(renderer, &boardShadow);
    // Board border
    SDL_Rect boardBorder = {boardOffset - 3, boardOffset - 3, 8 * squareSize + 6, 8 * squareSize + 6};
    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
    SDL_RenderFillRect(renderer, &boardBorder);
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int x = col * squareSize + boardOffset;
            int y = row * squareSize + boardOffset;
            SDL_Rect rect = {x, y, squareSize, squareSize};
            // Base square color
            if ((row + col) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, 240, 217, 181, 255); // Light square
            } else {
                SDL_SetRenderDrawColor(renderer, 181, 136, 99, 255); // Dark square
            }
            SDL_RenderFillRect(renderer, &rect);
            // Hover effect
            if (hoverRow == row && hoverCol == col && !promotionPossible) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 100, 60);
                SDL_RenderFillRect(renderer, &rect);
            }
            // Check highlight
            if ((whiteKingCheckRow == row && whiteKingCheckCol == col) ||
                (blackKingCheckRow == row && blackKingCheckCol == col)) {
                SDL_SetRenderDrawColor(renderer, 255, 50, 50, 180);
                SDL_RenderFillRect(renderer, &rect);
                // Pulsing border
                SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
                for (int i = 0; i < 3; i++) {
                    SDL_Rect borderRect = {x + i, y + i, squareSize - 2*i, squareSize - 2*i};
                    SDL_RenderDrawRect(renderer, &borderRect);
                }
            }
            // Selection highlight
            if (pieceSelected && selectedRow == row && selectedCol == col) {
                SDL_SetRenderDrawColor(renderer, 100, 200, 100, 120);
                SDL_RenderFillRect(renderer, &rect);
                SDL_SetRenderDrawColor(renderer, 50, 150, 50, 255);
                for (int i = 0; i < 4; i++) {
                    SDL_Rect borderRect = {x + i, y + i, squareSize - 2*i, squareSize - 2*i};
                    SDL_RenderDrawRect(renderer, &borderRect);
                }
            }
        }
    }
}
void Game::drawCoordinates() {
    SDL_Color coordColor = {200, 200, 200, 255};
    // Files (a-h)
    for (int col = 0; col < 8; col++) {
        char file = 'a' + col;
        std::string fileStr(1, file);
        int x = col * squareSize + boardOffset + squareSize / 2 - 5;
        drawText(fileStr, x, boardOffset - 25, coordColor, fontCoords);
        drawText(fileStr, x, boardOffset + 8 * squareSize + 8, coordColor, fontCoords);
    }
    // Ranks (1-8)
    for (int row = 0; row < 8; row++) {
        std::string rank = std::to_string(8 - row);
        int y = row * squareSize + boardOffset + squareSize / 2 - 8;
        drawText(rank, boardOffset - 25, y, coordColor, fontCoords);
        drawText(rank, boardOffset + 8 * squareSize + 10, y, coordColor, fontCoords);
    }
}
void Game::drawLegalMoveIndicators() {
    if (!pieceSelected) return;
    std::vector<Move> legalMoves = board.getLegalMoves(selectedRow, selectedCol);
    for (const Move& move : legalMoves) {
        int x = move.toCol * squareSize + boardOffset + squareSize / 2;
        int y = move.toRow * squareSize + boardOffset + squareSize / 2;
        bool isCapture = (board.state[move.toRow][move.toCol] != 0);
        if (isCapture) {
            // Capture indicator - ring
            SDL_SetRenderDrawColor(renderer, 255, 100, 100, 180);
            for (int r = 35; r < 42; r++) {
                for (int angle = 0; angle < 360; angle += 5) {
                    int px = x + r * cos(angle * M_PI / 180);
                    int py = y + r * sin(angle * M_PI / 180);
                    SDL_RenderDrawPoint(renderer, px, py);
                }
            }
        } else {
            // Move indicator - circle
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 150);
            for (int r = 0; r < 12; r++) {
                for (int angle = 0; angle < 360; angle += 5) {
                    int px = x + r * cos(angle * M_PI / 180);
                    int py = y + r * sin(angle * M_PI / 180);
                    SDL_RenderDrawPoint(renderer, px, py);
                }
            }
        }
    }
}
void Game::drawPieces() {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int piece = board.state[row][col];
            if (piece != 0 && piece != 7 && piece != -7) {
                if (pieceTextures.find(piece) != pieceTextures.end()) {
                    int x = col * squareSize + boardOffset + (squareSize - pieceSize) / 2;
                    int y = row * squareSize + boardOffset + (squareSize - pieceSize) / 2;
                    // Draw shadow
                    drawShadow(x, y, pieceSize);
                    // Draw piece
                    SDL_Rect destRect = {x, y, pieceSize, pieceSize};
                    SDL_RenderCopy(renderer, pieceTextures[piece], nullptr, &destRect);
                }
            }
        }
    }
}
void Game::drawPromotionDialog() {
    // Semi-transparent overlay
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect overlay = {0, 0, screenSize, screenSize};
    SDL_RenderFillRect(renderer, &overlay);
    // Dialog box with shadow
    SDL_Rect dialogShadow = {screenSize / 4 + 5, screenSize / 2 - 115, screenSize / 2, 240};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
    SDL_RenderFillRect(renderer, &dialogShadow);
    SDL_Rect dialogRect = {screenSize / 4, screenSize / 2 - 120, screenSize / 2, 240};
    SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
    SDL_RenderFillRect(renderer, &dialogRect);
    // Border
    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
    for (int i = 0; i < 3; i++) {
        SDL_Rect borderRect = {dialogRect.x + i, dialogRect.y + i,
                               dialogRect.w - 2*i, dialogRect.h - 2*i};
        SDL_RenderDrawRect(renderer, &borderRect);
    }
    // Title
    SDL_Color titleColor = {50, 50, 50, 255};
    drawText("Choose Promotion", screenSize / 2 - 100, screenSize / 2 - 100, titleColor, fontLarge);
    // Pieces
    int pieces[] = {2, 3, 4, 5};
    for (int i = 0; i < 4; i++) {
        int x = screenSize / 4 + 60 + i * 110;
        int y = screenSize / 2 - 120 + 60;
        // Hover effect
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        if (mouseX >= x && mouseX <= x + pieceSize &&
            mouseY >= y && mouseY <= y + pieceSize) {
            SDL_Rect hoverRect = {x - 5, y - 5, pieceSize + 10, pieceSize + 10};
            SDL_SetRenderDrawColor(renderer, 100, 200, 255, 100);
            SDL_RenderFillRect(renderer, &hoverRect);
        }
        SDL_Rect pieceRect = {x, y, pieceSize, pieceSize};
        SDL_RenderCopy(renderer, pieceTextures[pieces[i]], nullptr, &pieceRect);
    }
}
void Game::drawGameResult() {
    // Result box with shadow
    SDL_Rect resultShadow = {screenSize / 4 + 3, 18, screenSize / 2, 60};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);
    SDL_RenderFillRect(renderer, &resultShadow);
    SDL_Rect resultRect = {screenSize / 4, 15, screenSize / 2, 60};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &resultRect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int i = 0; i < 2; i++) {
        SDL_Rect borderRect = {resultRect.x + i, resultRect.y + i,
                               resultRect.w - 2*i, resultRect.h - 2*i};
        SDL_RenderDrawRect(renderer, &borderRect);
    }
    std::string resultText;
    SDL_Color textColor;
    if (whiteWin) {
        resultText = "White Wins!";
        textColor = {50, 150, 50, 255};
    } else if (blackWin) {
        resultText = "Black Wins!";
        textColor = {150, 50, 50, 255};
    } else {
        resultText = "Stalemate";
        textColor = {100, 100, 100, 255};
    }
    drawText(resultText, screenSize / 2 - 80, 30, textColor, fontLarge);
}
void Game::cleanup() {
    for (auto& [piece, texture] : pieceTextures) {
        SDL_DestroyTexture(texture);
    }
    pieceTextures.clear();
    if (font) TTF_CloseFont(font);
    if (fontLarge) TTF_CloseFont(fontLarge);
    if (fontCoords) TTF_CloseFont(fontCoords);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
void Game::run() {
    if (!init()) {
        std::cerr << "Failed to initialize!" << std::endl;
        return;
    }
    bool wCheck, bCheck;
    board.kingCheckCheck(wCheck, bCheck, whiteKingCheckRow, whiteKingCheckCol,
                        blackKingCheckRow, blackKingCheckCol);
    while (running) {
        handleEvents();
        update();
        draw();
        SDL_Delay(16);
    }
    cleanup();
}