#include <SFML/Graphics.hpp>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include "GameWindow.h"
#include "Leaderboard.h" // Include the leaderboard header

// Define constants for the board size and mine count
const int TILE_SIZE = 32;
const int UI_HEIGHT = 100;
int theRows, theCols, bombies;
bool theGameOver = false;
bool isNapTime = false;
bool isXrayMode = false;
bool firstBoom = true; // Flag to check if the first click is made
bool leaderboardShowed = false; // Flag to check if the leaderboard window is open
int flagLeft;
bool leaderboardPop = false; // Flag to ensure leaderboard is shown only once after win
sf::Time napDuration; // Time elapsed when the game is paused
sf::Time clockyClock; // Total time elapsed before pause

// Define the tile structure
struct TheTile {
    sf::Sprite theSprite;
    bool gotBomb;
    bool seeIt;
    bool lilFlag;
    int nextToBoom;
    const sf::Texture* originalTexture; // Pointer to store the original texture
};

// Function to read the configuration file
void loadConfig() {
    std::ifstream theConfig("files/config.cfg");
    if (theConfig.is_open()) {
        theConfig >> theCols >> theRows >> bombies;
        theConfig.close();
    } else {
        std::cerr << "Oopsie opening config file!" << std::endl;
    }
}

// Function to initialize the board
void initializeBoard(std::vector<std::vector<TheTile>>& daBoard, sf::Texture& faceDown) {
    daBoard.resize(theRows, std::vector<TheTile>(theCols));
    for (int i = 0; i < theRows; ++i) {
        for (int j = 0; j < theCols; ++j) {
            daBoard[i][j].theSprite.setTexture(faceDown);
            daBoard[i][j].theSprite.setPosition(j * TILE_SIZE, i * TILE_SIZE); // No offset for the UI
            daBoard[i][j].gotBomb = false;
            daBoard[i][j].seeIt = false;
            daBoard[i][j].lilFlag = false;
            daBoard[i][j].nextToBoom = 0;
            daBoard[i][j].originalTexture = &faceDown; // Initialize the original texture
        }
    }
    flagLeft = bombies;
}

// Function to load textures
void loadTextures(sf::Texture& faceDown, sf::Texture& faceUp, sf::Texture& boomBoom, sf::Texture& flappyFlag, sf::Texture& smileyFace, sf::Texture& winnerFace, sf::Texture& sadFace, sf::Texture& peekaboo, sf::Texture& napMode, sf::Texture& playMode, sf::Texture& numDigits, sf::Texture& leaderFace, std::vector<sf::Texture>& numTiles) {
    faceDown.loadFromFile("files/images/tile_hidden.png");
    faceUp.loadFromFile("files/images/tile_revealed.png");
    boomBoom.loadFromFile("files/images/mine.png");
    flappyFlag.loadFromFile("files/images/flag.png");
    smileyFace.loadFromFile("files/images/face_happy.png");
    winnerFace.loadFromFile("files/images/face_win.png");
    sadFace.loadFromFile("files/images/face_lose.png");
    peekaboo.loadFromFile("files/images/debug.png");
    napMode.loadFromFile("files/images/pause.png");
    playMode.loadFromFile("files/images/play.png");
    numDigits.loadFromFile("files/images/digits.png");
    leaderFace.loadFromFile("files/images/leaderboard.png"); // Load leaderboard button texture

    for (int i = 1; i <= 8; i++) {
        sf::Texture texture;
        texture.loadFromFile("files/images/number_" + std::to_string(i) + ".png");
        numTiles.push_back(texture);
    }
}

// Function to set mine positions
void placeMines(std::vector<std::vector<TheTile>>& daBoard) {
    srand(time(nullptr));
    for (int i = 0; i < bombies; i++) {
        int x, y;
        do {
            x = rand() % theCols;
            y = rand() % theRows;
        } while (daBoard[y][x].gotBomb);
        daBoard[y][x].gotBomb = true;
    }
}

// Function to calculate adjacent mines
void calculateAdjacentMines(std::vector<std::vector<TheTile>>& daBoard) {
    for (int i = 0; i < theRows; i++) {
        for (int j = 0; j < theCols; j++) {
            if (daBoard[i][j].gotBomb) continue;
            int mineCount = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int ny = i + dy;
                    int nx = j + dx;
                    if (ny >= 0 && ny < theRows && nx >= 0 && nx < theCols && daBoard[ny][nx].gotBomb) {
                        mineCount++;
                    }
                }
            }
            daBoard[i][j].nextToBoom = mineCount;
        }
    }
}

// Function to reveal a tile
void revealTile(std::vector<std::vector<TheTile>>& daBoard, int x, int y, sf::Texture& faceUp, std::vector<sf::Texture>& numTiles) {
    if (x < 0 || x >= theCols || y < 0 || y >= theRows || daBoard[y][x].seeIt || daBoard[y][x].lilFlag) return;
    daBoard[y][x].seeIt = true;
    daBoard[y][x].theSprite.setTexture(faceUp);

    if (daBoard[y][x].nextToBoom > 0) {
        daBoard[y][x].theSprite.setTexture(numTiles[daBoard[y][x].nextToBoom - 1]);
    } else {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int ny = y + dy;
                int nx = x + dx;
                if (ny >= 0 && ny < theRows && nx >= 0 && nx < theCols) {
                    revealTile(daBoard, nx, ny, faceUp, numTiles);
                }
            }
        }
    }
}

// Function to check if the player has won
bool checkWin(const std::vector<std::vector<TheTile>>& daBoard) {
    for (const auto& row : daBoard) {
        for (const auto& tile : row) {
            if (!tile.gotBomb && !tile.seeIt) {
                return false;
            }
        }
    }
    return true;
}

void flagAllBombs(std::vector<std::vector<TheTile>>& daBoard, sf::Texture& flappyFlag) {
    for (auto& row : daBoard) {
        for (auto& tile : row) {
            if (tile.gotBomb && !tile.lilFlag) {
                tile.lilFlag = true;
                tile.theSprite.setTexture(flappyFlag);
                flagLeft--;
            }
        }
    }
}

// Function to handle left click
void handleLeftClick(std::vector<std::vector<TheTile>>& daBoard, sf::Texture& flappyFlag, int x, int y, sf::Texture& faceUp, sf::Texture& boomBoom, std::vector<sf::Texture>& numTiles, sf::Texture& winnerFace, sf::Texture& sadFace, sf::Sprite& smileyButton, sf::Clock& gameClock, const std::string& playerDude) {
    if (leaderboardShowed) return;
    if (daBoard[y][x].lilFlag || isNapTime || theGameOver || leaderboardShowed) return;
    if (firstBoom) {
        gameClock.restart();
        firstBoom = false;
    }
    if (daBoard[y][x].gotBomb) {
        for (auto& row : daBoard) {
            for (auto& tile : row) {
                if (tile.gotBomb) {
                    tile.theSprite.setTexture(boomBoom);
                }
            }
        }
        smileyButton.setTexture(sadFace);
        theGameOver = true;
        // Game over logic
    } else {
        revealTile(daBoard, x, y, faceUp, numTiles);
        if (checkWin(daBoard)) {
            smileyButton.setTexture(winnerFace);
            theGameOver = true;
            flagAllBombs(daBoard, flappyFlag); // Flag all bombs when the game is won
        }
    }
}

// Function to handle right click
void handleRightClick(std::vector<std::vector<TheTile>>& daBoard, int x, int y, sf::Texture& flappyFlag, sf::Texture& faceDown) {
    if (leaderboardShowed) return;
    if (daBoard[y][x].seeIt || isNapTime || theGameOver || leaderboardShowed) return;
    daBoard[y][x].lilFlag = !daBoard[y][x].lilFlag;
    daBoard[y][x].theSprite.setTexture(daBoard[y][x].lilFlag ? flappyFlag : faceDown);
    flagLeft += daBoard[y][x].lilFlag ? -1 : 1;
}

// Function to toggle debug mode
void toggleDebugMode(std::vector<std::vector<TheTile>>& daBoard, sf::Texture& boomBoom, sf::Texture& faceDown) {
    isXrayMode = !isXrayMode;
    for (auto& row : daBoard) {
        for (auto& tile : row) {
            if (tile.gotBomb && !tile.seeIt && !tile.lilFlag) {
                tile.theSprite.setTexture(isXrayMode ? boomBoom : faceDown);
            }
        }
    }
}

// Function to update the counter display
void updateCounter(std::vector<sf::Sprite>& counterSprites, int value, sf::Texture& numDigits) {
    std::string str = std::to_string(value);
    if (value < 0) str = "-" + std::to_string(-value);
    if (str.length() > 4) str = str.substr(str.length() - 4); // Ensure it doesn't exceed 4 digits
    while (str.length() < 4) str = "0" + str; // Pad with leading zeros to ensure 4 digits

    for (size_t i = 0; i < counterSprites.size(); i++) {
        int digit = str[i] == '-' ? 10 : str[i] - '0';
        counterSprites[i].setTextureRect(sf::IntRect(digit * 21, 0, 21, 32));
    }
}

// Function to reset the game
void resetGame(std::vector<std::vector<TheTile>>& daBoard, sf::Texture& faceDown, sf::Texture& smileyFace, sf::Sprite& smileyButton, sf::Clock& gameClock) {
    initializeBoard(daBoard, faceDown);
    placeMines(daBoard);
    calculateAdjacentMines(daBoard);
    smileyButton.setTexture(smileyFace);
    theGameOver = false;
    isNapTime = false;
    isXrayMode = false;
    firstBoom = true;
    leaderboardPop = false; // Reset the leaderboard shown flag
    gameClock.restart();
    clockyClock = sf::Time::Zero; // Reset the total elapsed time
    flagLeft = bombies;
}

// Function to pause and reveal tiles
void pauseAndReveal(std::vector<std::vector<TheTile>>& daBoard, sf::Texture& faceUp) {
    for (auto& row : daBoard) {
        for (auto& tile : row) {
            tile.originalTexture = tile.theSprite.getTexture();
            tile.theSprite.setTexture(faceUp);
        }
    }
}

// Function to resume and restore tiles
void resumeAndRestore(std::vector<std::vector<TheTile>>& daBoard) {
    for (auto& row : daBoard) {
        for (auto& tile : row) {
            tile.theSprite.setTexture(*tile.originalTexture);
        }
    }
}

void showGameWindow(const std::string &playerDude) {
    // Load configuration
    loadConfig();

    sf::RenderWindow gameWindow(sf::VideoMode(theCols * TILE_SIZE, theRows * TILE_SIZE + UI_HEIGHT), "Minesweeper", sf::Style::Close);
    gameWindow.clear(sf::Color::White); // Set background color to white

    // Load textures
    sf::Texture faceDown, faceUp, boomBoom, flappyFlag, smileyFace, winnerFace, sadFace, peekaboo, napMode, playMode, numDigits, leaderFace;
    std::vector<sf::Texture> numTiles;
    loadTextures(faceDown, faceUp, boomBoom, flappyFlag, smileyFace, winnerFace, sadFace, peekaboo, napMode, playMode, numDigits, leaderFace, numTiles);

    // Initialize board
    std::vector<std::vector<TheTile>> daBoard;
    initializeBoard(daBoard, faceDown);

    // Place mines and calculate adjacent mines
    placeMines(daBoard);
    calculateAdjacentMines(daBoard);

    // UI elements
    sf::Sprite smileyButton(smileyFace);
    smileyButton.setPosition((theCols * TILE_SIZE) / 2 - 20, theRows * TILE_SIZE + 20);

    sf::Sprite peekabooButton(peekaboo);
    peekabooButton.setPosition((theCols * TILE_SIZE) / 2 + 60, theRows * TILE_SIZE + 20);

    sf::Sprite napButton(napMode);
    napButton.setPosition((theCols * TILE_SIZE) / 2 + 150, theRows * TILE_SIZE + 20);

    sf::Sprite leaderButton(leaderFace); // Add leaderboard button
    leaderButton.setPosition((theCols * TILE_SIZE) / 2 + 240, theRows * TILE_SIZE + 20);

    // Counter for remaining flags
    std::vector<sf::Sprite> counterSprites(4, sf::Sprite(numDigits));
    for (size_t i = 0; i < counterSprites.size(); i++) {
        counterSprites[i].setPosition(20 + i * 21, theRows * TILE_SIZE + 20);
    }

    // Timer
    std::vector<sf::Sprite> timerSprites(4, sf::Sprite(numDigits));
    for (size_t i = 0; i < timerSprites.size(); i++) {
        timerSprites[i].setPosition((theCols * TILE_SIZE) - 80 + i * 21, theRows * TILE_SIZE + 20);
    }

    sf::Clock gameClock;
    int elapsedTime = 0;

    // Main game loop
    while (gameWindow.isOpen()) {
        sf::Event event;
        while (gameWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                gameWindow.close();
            } else if (event.type == sf::Event::MouseButtonPressed && !leaderboardShowed) {
                int x = event.mouseButton.x / TILE_SIZE;
                int y = event.mouseButton.y / TILE_SIZE;
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (event.mouseButton.x >= smileyButton.getPosition().x && event.mouseButton.x <= smileyButton.getPosition().x + smileyButton.getGlobalBounds().width && event.mouseButton.y >= smileyButton.getPosition().y && event.mouseButton.y <= smileyButton.getPosition().y + smileyButton.getGlobalBounds().height) {
                        resetGame(daBoard, faceDown, smileyFace, smileyButton, gameClock);
                        isNapTime = false;
                    } else if (event.mouseButton.x >= peekabooButton.getPosition().x && event.mouseButton.x <= peekabooButton.getPosition().x + peekabooButton.getGlobalBounds().width && event.mouseButton.y >= peekabooButton.getPosition().y && event.mouseButton.y <= peekabooButton.getPosition().y + peekabooButton.getGlobalBounds().height) {
                        if (!theGameOver) {
                            toggleDebugMode(daBoard, boomBoom, faceDown);
                        }
                    } else if (event.mouseButton.x >= napButton.getPosition().x && event.mouseButton.x <= napButton.getPosition().x + napButton.getGlobalBounds().width && event.mouseButton.y >= napButton.getPosition().y && event.mouseButton.y <= napButton.getPosition().y + napButton.getGlobalBounds().height) {
                        if (isNapTime) {
                            gameClock.restart();
                            clockyClock += napDuration;
                            resumeAndRestore(daBoard);
                            isNapTime = false;
                            napButton.setTexture(napMode);
                        } else {
                            napDuration = gameClock.getElapsedTime();
                            isNapTime = true;
                            pauseAndReveal(daBoard, faceUp);
                            napButton.setTexture(playMode);
                        }
                    } else if (event.mouseButton.x >= leaderButton.getPosition().x && event.mouseButton.x <= leaderButton.getPosition().x + leaderButton.getGlobalBounds().width && event.mouseButton.y >= leaderButton.getPosition().y && event.mouseButton.y <= leaderButton.getPosition().y + leaderButton.getGlobalBounds().height) {
                        if (!leaderboardShowed) {
                            if (!isNapTime) {
                                napDuration = gameClock.getElapsedTime();
                                clockyClock += napDuration;
                                isNapTime = true;
                                napButton.setTexture(playMode);
                            }
                            leaderboardShowed = true;
                            gameWindow.setVisible(false); // Hide the main game window
                            sf::RenderWindow leaderboardWindow(sf::VideoMode(400, 400), "Leaderboard", sf::Style::Titlebar | sf::Style::Close);
                            showLeaderboardWindow(leaderboardWindow, false, 0, playerDude, isNapTime, napButton, napMode);
                            leaderboardShowed = false;
                            gameWindow.setVisible(true); // Show the main game window again
                            if (isNapTime) {
                                gameClock.restart();
                            } else {
                                clockyClock += napDuration;
                                gameClock.restart();
                            }
                            isNapTime = false;
                        }
                    } else if (!isNapTime && !theGameOver) {
                        handleLeftClick(daBoard, flappyFlag, x, y, faceUp, boomBoom, numTiles, winnerFace, sadFace, smileyButton, gameClock, playerDude);
                    }
                } else if (event.mouseButton.button == sf::Mouse::Right && !leaderboardShowed && !isNapTime && !theGameOver) {
                    handleRightClick(daBoard, x, y, flappyFlag, faceDown);
                }
            }
        }

        if (!isNapTime && !theGameOver && !firstBoom && !leaderboardShowed) {
            elapsedTime = clockyClock.asSeconds() + gameClock.getElapsedTime().asSeconds();
        }

        updateCounter(counterSprites, flagLeft, numDigits);
        updateCounter(timerSprites, elapsedTime, numDigits);

        gameWindow.clear(sf::Color::White);
        for (auto& row : daBoard) {
            for (auto& tile : row) {
                gameWindow.draw(tile.theSprite);
            }
        }
        gameWindow.draw(smileyButton);
        gameWindow.draw(peekabooButton);
        gameWindow.draw(napButton);
        gameWindow.draw(leaderButton); // Draw leaderboard button
        for (auto& sprite : counterSprites) {
            gameWindow.draw(sprite);
        }
        for (auto& sprite : timerSprites) {
            gameWindow.draw(sprite);
        }
        gameWindow.display();

        if (theGameOver && checkWin(daBoard) && !leaderboardPop) {
            leaderboardPop = true;
            leaderboardShowed = true;
            gameWindow.setVisible(false); // Hide the main game window
            sf::RenderWindow leaderboardWindow(sf::VideoMode(400, 400), "Leaderboard", sf::Style::Titlebar | sf::Style::Close);
            showLeaderboardWindow(leaderboardWindow, true, elapsedTime, playerDude, isNapTime, napButton, napMode);
            leaderboardShowed = false;
            gameWindow.setVisible(true); // Show the main game window again
        }
    }
}