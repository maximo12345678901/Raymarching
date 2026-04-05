#include "../vec.h"
#include "../ui.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <cmath>
#include <omp.h>
#include <variant>
#include <array>
#include <algorithm>
#include <limits>
#include "SFML/Window/Keyboard.hpp"

uint windowWidth = 1600;
uint windowHeight = 900;
float aspectRatio;
int resolutionWidth = 320;
int resolutionHeight;
float renderScale;

struct Ray {
    Vector3 position;
    Vector3 direction;  

    Ray(Vector3 pos_, Vector3 dir_) {
        position = pos_;
        direction = dir_;
    }
};

float CircleSDF(Vector3 pos, float r) {
    return Vector3::length(pos) - r;
}

float BoxSDF(Vector3 p, Vector3 b) {
    Vector3 q = Vector3::abs(p) - b;
    return Vector3::length(Vector3::max(q, 0.0)) + std::min(std::max(q.x, std::min(q.y, q.z)), 0.0);
}

float TorusSDF(Vector3 p, Vector2 t) {
     Vector2 q(Vector3::length(Vector3(p.x, 0, p.z)) - t.x, p.y);
    return Vector2::length(q) - t.y;
}

float BoxFrameSDF(Vector3 p, Vector3 b, float e) {
    p = Vector3::abs(p) - b;
    Vector3 q = Vector3::abs(p + Vector3(e, e, e)) - Vector3(e, e, e);
    return std::min({
        Vector3::length(Vector3::max(Vector3(p.x, q.y, q.z), 0.0)) + std::min(std::max(p.x, std::max(q.y, q.z)), 0.0),
        Vector3::length(Vector3::max(Vector3(q.x, p.y, q.z), 0.0)) + std::min(std::max(q.x, std::max(p.y, q.z)), 0.0),
        Vector3::length(Vector3::max(Vector3(q.x, q.y, p.z), 0.0)) + std::min(std::max(q.x, std::max(q.y, p.z)), 0.0)
    });
}

float CapsuleSDF(Vector3 p, Vector3 a, Vector3 b, float r) {
    Vector3 ab = b - a;
    Vector3 ap = p - a;
    double t = Vector3::dot(ap, ab) / Vector3::dot(ab, ab);
    t = std::max(0.0, std::min(1.0, t));
    Vector3 closest = a + ab * t;
    return Vector3::length(p - closest) - r;
}

float CustomSDF(Vector3 p, Vector2 t) {
    Vector2 q(Vector3::length(Vector3(p.z, 0, p.z)) - t.x, p.y);
    return Vector2::length(q) - t.y;

}


float Scene(Vector3 p) {
    auto pmod = [](double a, double b) { return std::fmod(std::fmod(a, b) + b, b); };

    float d = BoxSDF(Vector3(0, 0, 0) - p, Vector3(1, 1, 1));
    return d;
}

int main() {
    aspectRatio = (float) windowWidth / (float) windowHeight;
    resolutionHeight = (float) resolutionWidth / aspectRatio;
    renderScale = (float) windowWidth / (float) resolutionWidth;

    sf::Image image(sf::Vector2u(resolutionWidth, resolutionHeight));
    sf::Texture texture;
    (void)texture.loadFromImage(image);
    sf::Sprite sprite(texture);
    sprite.setScale({renderScale, renderScale});

    Vector3 camPos(0.0, 0.0, -5.0);
    float moveSpeed = 0.1f;
    Vector2 camRot(0.0, 0.0);
    float turnSpeed = 0.1f;
    float fov = M_PI/2.0f;
    
    sf::RenderWindow window(sf::VideoMode({windowWidth, windowHeight}), "reymarc");
    window.setFramerateLimit(60);
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            camPos += Vector3::getForward(camRot) * moveSpeed; 
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            camPos -= Vector3::getForward(camRot) * moveSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            camPos += Vector3::getForward(Vector2(camRot.x, camRot.y + M_PI / 2.0f)) * moveSpeed; 
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            camPos -= Vector3::getForward(Vector2(camRot.x, camRot.y + M_PI / 2.0f)) * moveSpeed;
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

        if (camRot.x <= -M_PI / 2.0f) {
            camRot.x = -M_PI / 2.0f + 0.01f;
        }
        else if (camRot.x >= M_PI / 2.0f) {
            camRot.x = M_PI / 2.0f - 0.01f;
        }

        std::cout << "\033[2J";
        camPos.say();
        camRot.say();

        Vector3 forward = Vector3::getForward(camRot);
        Vector3 worldUp(0.0, 1.0, 0.0);
        Vector3 right = Vector3::normalize(Vector3::cross(worldUp, forward));
        Vector3 up    = Vector3::normalize(Vector3::cross(forward, right));

        float halfFovTan = std::tan(fov / 2.0f);

        #pragma omp parallel for collapse(2)
        for (unsigned x = 0; x < resolutionWidth; ++x) {
            for (unsigned y = 0; y < resolutionHeight; ++y) {

                float ndcX =  (x + 0.5f) / resolutionWidth  * 2.0f - 1.0f;
                float ndcY = -((y + 0.5f) / resolutionHeight * 2.0f - 1.0f);

                Vector3 rayDir = Vector3::normalize(
                    forward
                    + right * (ndcX * halfFovTan * aspectRatio)
                    + up    * (ndcY * halfFovTan)
                );

                Ray ray(camPos, rayDir);

                sf::Color px = sf::Color::Black;
                for (int i = 0; i < 1000; i++) {
                    float dist = Scene(ray.position);
                    ray.position += ray.direction * dist;

                    if (dist < 0.01f) {
                        float distFromCamera = Vector3::distance(camPos, ray.position);
                        float color = std::min(255.0f, 2000.0f / (distFromCamera + 0.1f));
                        px = sf::Color(color, color, color);
                        break;
                    }
                    if (dist > 50) {
                        break;
                    }
                }
                image.setPixel({x, y}, px);
            }
        }

        (void)texture.loadFromImage(image);
        window.clear();
        window.draw(sprite);
        window.display();
    }
    return 0;
}
