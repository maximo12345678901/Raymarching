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

int windowWidth = 1600;
int windowHeight = 900;
int resolutionWidth = 160;

int main() {
    sf::RenderWindow window(sf::VideoMode({800u, 600u}), "reymarc");
    return 0;
}
