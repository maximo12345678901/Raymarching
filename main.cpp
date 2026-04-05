// main.cpp
#include "../vec.h"
#include "../ui.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include "SFML/Window/Keyboard.hpp"

uint windowWidth = 2000;
uint windowHeight = 1000;
int resolutionWidth = 2000;
int resolutionHeight;
float renderScale;

int main() {
    float aspectRatio = (float)windowWidth / (float)windowHeight;
    resolutionHeight = (float)resolutionWidth / aspectRatio;
    renderScale = (float)windowWidth / (float)resolutionWidth;

    Vector3 camPos(0.0, 0.0, -5.0);
    float moveSpeed = 0.1f;
    Vector2 camRot(0.0, 0.0);
    float turnSpeed = 0.05f;
    float fov = M_PI / 2.0f;

    sf::RenderWindow window(sf::VideoMode({windowWidth, windowHeight}), "reymarc");
    window.setFramerateLimit(60);

    sf::Shader shader;
    if (!shader.loadFromFile("raymarch.frag", sf::Shader::Type::Fragment)) {
        return -1;
    }

    sf::RenderTexture renderTexture(sf::Vector2u(resolutionWidth, resolutionHeight));

    sf::RectangleShape quad(sf::Vector2f(resolutionWidth, resolutionHeight));

    sf::Sprite sprite(renderTexture.getTexture());
    sprite.setScale({renderScale, renderScale});

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        Vector3 forward = Vector3::getForward(camRot);
        Vector3 worldUp(0.0, 1.0, 0.0);
        Vector3 right = Vector3::normalize(Vector3::cross(worldUp, forward));
        Vector3 up    = Vector3::normalize(Vector3::cross(forward, right));

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            camPos += forward * moveSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            camPos -= forward * moveSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            camPos += right * moveSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            camPos -= right * moveSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
            camPos.y += moveSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
            camPos.y -= moveSpeed;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            camRot.y -= turnSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            camRot.y += turnSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            camRot.x -= turnSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            camRot.x += turnSpeed;

        if (camRot.x <= -M_PI / 2.0f)
            camRot.x = -M_PI / 2.0f + 0.01f;
        else if (camRot.x >= M_PI / 2.0f)
            camRot.x = M_PI / 2.0f - 0.01f;

        std::cout << "\033[2J";
        camPos.say();
        camRot.say();

        shader.setUniform("uResolution", sf::Glsl::Vec2(resolutionWidth, resolutionHeight));
        shader.setUniform("uCamPos",     sf::Glsl::Vec3(camPos.x, camPos.y, camPos.z));
        shader.setUniform("uForward",    sf::Glsl::Vec3(forward.x, forward.y, forward.z));
        shader.setUniform("uRight",      sf::Glsl::Vec3(right.x, right.y, right.z));
        shader.setUniform("uUp",         sf::Glsl::Vec3(up.x, up.y, up.z));
        shader.setUniform("uFov",        fov);

        renderTexture.clear();
        renderTexture.draw(quad, &shader);
        renderTexture.display();

        window.clear();
        window.draw(sprite);
        window.display();
    }
    return 0;
}
