#include <SFML/Graphics.hpp>

#include "atomicQueue.hpp"
#include "sierpinskiPolygon.hpp"

#include <math.h>
#include <future>
#include <random>
#include <algorithm>

std::atomic_bool running = true;
atomicQueue<sf::Vector2f> queue;

/* Fills the queue with points.
 */
void fillQueue(const std::vector<sf::Vector2f>& vertices, const sf::Vector2f& initialPoint) {
  size_t verticesCount = vertices.size();

  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_int_distribution<int> uid(0, verticesCount - 1);

  sf::Vector2f p = initialPoint;
  while (running) {
    int side = uid(rng);
    sf::Vector2f a = vertices[side], b = vertices[(side + 1) % verticesCount];

    p = (p + a + b) / 3.f;
    while (!queue.push(p) && running) {}
  }
}

std::vector<sf::Vector2f> createVertices(size_t verticesCount = 6) {
  std::vector<sf::Vector2f> vertices;

  float circumradius = 100.f;
  float angleDelta = 360 / verticesCount;
  float angle = 90 + angleDelta / 2;

  for (size_t i = 0; i < verticesCount; i++) {
    vertices.push_back(sf::Vector2f(circumradius * cos(angle * M_PI / 180), circumradius * sin(angle * M_PI / 180)));
    angle += angleDelta;
  }

  return vertices;
}

enum class State { View, Move };

int main() {
  sf::RenderWindow window(sf::VideoMode(800, 600), "Koch Snowflake");

  size_t sides = 6;

  std::vector<sf::Vector2f> vertices = createVertices(sides);
  SierpinskiPolygon sierpinskiPolygon(vertices);
  sf::Vector2f initialPoint(0, 0);

  auto f = std::async(std::launch::async, fillQueue, vertices, initialPoint);

  float zoom = 0.4f;
  float maxZoom = 1.f;
  float minZoom = 0.001f;
  sf::View view = window.getDefaultView();
  view.setCenter(0.f, 0.f);
  view.zoom(zoom);
  window.setView(view);

  window.clear(sf::Color::Black);

  sf::Clock clock;
  sf::Vector2f oldPos;
  State state = State::View;
  while (window.isOpen()) {
    sf::Event e;
    while (window.pollEvent(e)) {
      if (e.type == sf::Event::Closed || (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape)) {
        running = false;
        window.close();
        break;
      }
      if (e.type == sf::Event::MouseWheelScrolled) {
        if (e.mouseWheelScroll.delta < 0) {
          zoom = std::min(maxZoom, zoom * 1.12f);
        }
        else if (e.mouseWheelScroll.delta > 0) {
          zoom = std::max(minZoom, zoom / 1.12f);
        }

        view.setSize(window.getDefaultView().getSize());
        view.zoom(zoom);
        window.setView(view);
      }
      switch (state) {
      case State::View:
        if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
          oldPos = sf::Vector2f(sf::Mouse::getPosition());
          state = State::Move;
        }
        else if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Space) {
          view.setCenter(0.f, 0.f);
          window.setView(view);
        }
        break;
      case State::Move:
        if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left) {
          state = State::View;
        }
        else {
          sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition());
          sf::Vector2f deltaPos = oldPos - mousePos;
          deltaPos.x *= zoom;
          deltaPos.y *= zoom;
          view.move(deltaPos);
          window.setView(view);
          oldPos = mousePos;
        }
        break;
      }
    }

    if (clock.getElapsedTime().asMicroseconds() > 1) {
      boost::optional<sf::Vector2f> p = queue.pop();
      if (p.has_value()) {
        sierpinskiPolygon.addPoint(p.value());
      }
    }

    window.clear();
    sierpinskiPolygon.draw(window);
    window.display();
  }
}