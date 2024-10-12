#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include "Leaderboard.h"
#include "CenterText.h" // Include the common header file

struct ScoreKeeper {
    int tickTock;
    std::string nickname;
    bool isNewbie;

    bool operator<(const ScoreKeeper &other) const {
        return tickTock < other.tickTock;
    }
};

void loadScores(std::vector<ScoreKeeper> &besties) {
    std::ifstream theFile("files/leaderboard.txt");
    if (theFile.is_open()) {
        std::string sentence;
        while (std::getline(theFile, sentence)) {
            std::istringstream daStream(sentence);
            std::string tickTockStr, nickname;
            std::getline(daStream, tickTockStr, ',');
            std::getline(daStream, nickname);

            ScoreKeeper daScore;
            daScore.tickTock = std::stoi(tickTockStr.substr(0, 2)) * 60 + std::stoi(tickTockStr.substr(3, 2));
            daScore.nickname = nickname;
            daScore.isNewbie = false;

            besties.push_back(daScore);
        }
        theFile.close();
    } else {
        std::cerr << "Oopsie opening leaderboard file!" << std::endl;
    }
}

void saveScores(const std::vector<ScoreKeeper> &besties) {
    std::ofstream theFile("files/leaderboard.txt");
    if (theFile.is_open()) {
        for (const auto &daScore : besties) {
            theFile << (daScore.tickTock / 60 < 10 ? "0" : "") << daScore.tickTock / 60 << ":"
                 << (daScore.tickTock % 60 < 10 ? "0" : "") << daScore.tickTock % 60 << ","
                 << daScore.nickname << std::endl;
        }
        theFile.close();
    } else {
        std::cerr << "Oopsie opening leaderboard file for writing!" << std::endl;
    }
}

void updateScores(std::vector<ScoreKeeper> &besties, int speedyGonzales, const std::string &playerDude) {
    ScoreKeeper newChampion{speedyGonzales, playerDude + "*", true};
    besties.push_back(newChampion);
    std::sort(besties.begin(), besties.end());
    if (besties.size() > 5) {
        besties.pop_back();
    }
    saveScores(besties);
}

void showLeaderboardWindow(sf::RenderWindow &boomboomWindow, bool youWinz, int speedyGonzales, const std::string &playerDude, bool &isNapTime, sf::Sprite &freezeButton, sf::Texture &napTexture) {
    std::vector<ScoreKeeper> besties;
    loadScores(besties);

    if (youWinz) {
        updateScores(besties, speedyGonzales, playerDude);
    }

    sf::Font daFont;
    if (!daFont.loadFromFile("files/font.ttf")) {
        std::cerr << "Oopsie loading font file!" << std::endl;
    }

    sf::Text daTitle("LEADERBOARD", daFont, 20);
    daTitle.setFillColor(sf::Color::White);
    daTitle.setStyle(sf::Text::Bold | sf::Text::Underlined);
    centerText(daTitle, 200, 30);

    std::string daContent;
    for (size_t i = 0; i < besties.size(); ++i) {
        const auto &daScore = besties[i];
        daContent += std::to_string(i + 1) + "\t" +
                   (daScore.tickTock / 60 < 10 ? "0" : "") + std::to_string(daScore.tickTock / 60) + ":" +
                   (daScore.tickTock % 60 < 10 ? "0" : "") + std::to_string(daScore.tickTock % 60) + "\t" +
                   daScore.nickname + "\n\n";
    }

    sf::Text daScoreText(daContent, daFont, 18);
    daScoreText.setStyle(sf::Text::Bold);
    daScoreText.setFillColor(sf::Color::White);
    centerText(daScoreText, 200, 200);

    while (boomboomWindow.isOpen()) {
        sf::Event event;
        while (boomboomWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                boomboomWindow.close();
                isNapTime = false; // Resume the game when the leaderboard is closed
                freezeButton.setTexture(napTexture); // Set the pause button back to the pause texture
            }
        }

        boomboomWindow.clear(sf::Color::Blue);
        boomboomWindow.draw(daTitle);
        boomboomWindow.draw(daScoreText);
        boomboomWindow.display();
    }
}