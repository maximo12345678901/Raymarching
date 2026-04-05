#include <SFML/Graphics.hpp>
#include <cmath>
#include <omp.h>
#include <variant>
#include <array>
#include <algorithm>
#include <limits>
#include "../vec.h"
#include "../ui.h"
#include "SFML/Window/Keyboard.hpp"

uint windowWidth = 1600;
uint windowHeight = 900;
float aspectRatio;
int resolutionWidth = 160;
int resolutionHeight;
float renderScale;

int main() {
    aspectRatio = (float) windowWidth / (float) windowHeight;
    resolutionHeight = (float) resolutionWidth / aspectRatio;
    renderScale = (float) windowWidth / (float) resolutionWidth;

    sf::Image image(sf::Vector2u(resolutionWidth, resolutionHeight));
    sf::Texture texture;
    sf::Sprite sprite(texture);
    sprite.setScale({renderScale, renderScale});
    
    sf::RenderWindow window(sf::VideoMode({windowWidth, windowHeight}), "reymarc");
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        for (unsigned x = 0; x < resolutionWidth; ++x) {
            for (unsigned y = 0; y < resolutionHeight; ++y) {
                image.setPixel({x, y}, sf::Color((x * 255)/resolutionWidth, (y * 255)/resolutionHeight, 0));
            }
        }
        (void)texture.loadFromImage(image);
        sprite = sf::Sprite(texture);
        sprite.setScale({renderScale, renderScale});

        window.clear();
        window.draw(sprite);
        window.display();
    }
    return 0;
}
