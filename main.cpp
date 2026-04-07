// main.cpp
#include "../vec.h"
#include "../ui.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include "SFML/Window/Keyboard.hpp"

uint windowWidth = 2000;
uint windowHeight = 1000;
int resolutionWidth = 1000;
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

    double blendStrength = 1.0;
    double mandelPower = 2.0;
    double mandelIterations = 10.0;
    double glowStrength = 0.05;
    double rayBendStrength = 0.1;
    Background background(sf::Color(20, 20, 20), sf::Vector2f(10, 10), sf::Vector2f(500, 100));

    sf::Font font("font.tff");
    
    // Blend strength slider
    Slider blendSlider(
    blendStrength,
    0.0001, 10.0,
    sf::Vector2f(10.f, 75.f),
    490.f,
    sf::Color(100, 100, 100),
    sf::Color::White,
    4.f,
    8.f,
    2,
    font,
    "Blend strength"
    );
    // Mandel power slider
    Slider mandelPowerSlider(
    mandelPower,
    1.0, 4.0,
    sf::Vector2f(10.f, 150.f),
    490.f,
    sf::Color(100, 100, 100),
    sf::Color::White,
    4.f,
    8.f,
    2,
    font,
    "Mandelbulb power"
    );
    // Mandel iterations slider;
    Slider iterationSlider(
    mandelIterations,
    5.0, 200.0,
    sf::Vector2f(10.f, 225.f),
    490.f,
    sf::Color(100, 100, 100),
    sf::Color::White,
    4.f,
    8.f,
    0,
    font,
    "Mandelbulb iterations"
    );
    // Glow slider
    Slider glowSlider(
    glowStrength,
    0.0001, 1.0,
    sf::Vector2f(10.f, 300.f),
    490.f,
    sf::Color(100, 100, 100),
    sf::Color::White,
    4.f,
    8.f,
    0,
    font,
    "glow strength"
    );
    // Ray bend strength slider
        // Glow slider
    Slider rayBendStrengthSlider(
    rayBendStrength,
    0.0, 1.0,
    sf::Vector2f(10.f, 375.f),
    490.f,
    sf::Color(100, 100, 100),
    sf::Color::White,
    4.f,
    8.f,
    0,
    font,
    "ray bend strength"
    );



    bool isShowingUI = true;
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
            blendSlider.HandleEvent(*event, window);
            mandelPowerSlider.HandleEvent(*event, window);
            iterationSlider.HandleEvent(*event, window);
            glowSlider.HandleEvent(*event, window);
            rayBendStrengthSlider.HandleEvent(*event, window);
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
        shader.setUniform("uBlendStrength", static_cast<float>(blendStrength));
        shader.setUniform("Iterations", static_cast<int>(mandelIterations));
        shader.setUniform("Power", static_cast<float>(mandelPower));
        shader.setUniform("GlowStrength", static_cast<float>(glowStrength));
        shader.setUniform("RayBendStrength", static_cast<float>(rayBendStrength));

        renderTexture.clear();
        renderTexture.draw(quad, &shader);
        renderTexture.display();

        window.clear();
        window.draw(sprite);
        if (isShowingUI) {
            background.Draw(window);
            blendSlider.Draw(window);
            mandelPowerSlider.Draw(window);
            iterationSlider.Draw(window);
            glowSlider.Draw(window);
            rayBendStrengthSlider.Draw(window);
        }
        window.display();
    }
    return 0;
}
