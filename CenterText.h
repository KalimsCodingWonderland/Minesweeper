//
// Created by kalim on 7/29/2024.
//

#ifndef CENTERTEXT_H
#define CENTERTEXT_H

#include <SFML/Graphics.hpp>

inline void centerText(sf::Text &text, float x, float y) {
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    text.setPosition(sf::Vector2f(x, y));
}

#endif //CENTERTEXT_H
