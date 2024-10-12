#include <iostream>
#include <SFML/Graphics.hpp>
#include <cctype>
#include <string>
#include <filesystem>
#include "WelcomeWindow.h"
#include "GameWindow.h"
#include "CenterText.h" // Assuming Utilities.h contains the centerText function

void showWelcomeWindow() {
    sf::RenderWindow funnyWindow(sf::VideoMode(800, 612), "Welcome to Minesweeper", sf::Style::Close);

    sf::Font goofyFont;

    if (!goofyFont.loadFromFile("files/font.ttf")) {
        std::cerr << "Error loading font file!" << std::endl;
    }

    sf::Text sillyWelcome("WELCOME TO MINESWEEPER!", goofyFont, 24); //Say whatup to the homies
    sillyWelcome.setFillColor(sf::Color::White);
    sillyWelcome.setStyle(sf::Text::Bold | sf::Text::Underlined);
    centerText(sillyWelcome, 400, 156);

    sf::Text sillyPrompt("Enter your name:", goofyFont, 20); //Ask for homies name
    sillyPrompt.setFillColor(sf::Color::White);
    sillyPrompt.setStyle(sf::Text::Bold);
    centerText(sillyPrompt, 400, 231);

    sf::Text sillyInput("", goofyFont, 18); //Get silly
    sillyInput.setFillColor(sf::Color::Yellow);
    sillyInput.setStyle(sf::Text::Bold);
    centerText(sillyInput, 400, 261);

    sf::RectangleShape cursor(sf::Vector2f(2, 24));  // Create a cursor shape
    cursor.setFillColor(sf::Color::Yellow);
    bool cursorVisible = true;
    sf::Clock clock;
    const sf::Time blinkInterval = sf::milliseconds(500); //BEHOLD THE BLINKER!

    std::string userInput;
    bool isRunning = true;
    size_t cursorPosition = 0;  // Track the cursor position within the text

    while (isRunning) {
        sf::Event event;
        while (funnyWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                funnyWindow.close();
                isRunning = false;
            }
            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == '\b' && cursorPosition > 0) {
                    userInput.erase(cursorPosition - 1, 1);
                    cursorPosition--;
                } else if (event.text.unicode >= 32 && event.text.unicode < 128 && userInput.size() < 10) {
                    char c = static_cast<char>(event.text.unicode);
                    if (std::isalpha(c)) {
                        userInput.insert(cursorPosition, 1, std::tolower(c));
                        cursorPosition++;
                    }
                }
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Enter && !userInput.empty()) {
                    isRunning = false;
                    funnyWindow.close();
                    showGameWindow(userInput); // Pass the player name to the game window
                }
            }
        }

        if (!userInput.empty()) {
            userInput[0] = std::toupper(userInput[0]);
            for (size_t i = 1; i < userInput.size(); ++i) {
                userInput[i] = std::tolower(userInput[i]);
            }
        }

        sillyInput.setString(userInput);
        centerText(sillyInput, 400, 250);

        if (clock.getElapsedTime() >= blinkInterval) {
            cursorVisible = !cursorVisible;
            clock.restart();
        }

        if (cursorVisible) {
            sf::FloatRect textBounds = sillyInput.getGlobalBounds();
            float cursorX = textBounds.left + textBounds.width;
            cursor.setPosition(cursorX, sillyInput.getPosition().y - 10);
        }

        funnyWindow.clear(sf::Color::Blue);
        funnyWindow.draw(sillyWelcome);
        funnyWindow.draw(sillyPrompt);
        funnyWindow.draw(sillyInput);
        if (cursorVisible) {
            funnyWindow.draw(cursor);
        }
        funnyWindow.display();
    }
}


