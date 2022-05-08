#include <SFML/Graphics.hpp>

#include "atomicQueue.hpp"

#include <math.h>
#include <array>
#include <future>
#include <random>

std::atomic_bool running = true;
atomicQueue<sf::Vector2i> queue;

/* Fills the queue with points.
 */
void fillQueue(const std::array<sf::Vector2i, 6>& hex, const sf::Vector2i& initialPoint) {
  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_int_distribution<int> uid(0, 5);

  sf::Vector2i p = initialPoint;
  while (running) {
    int side = uid(rng);
    sf::Vector2i a = hex[side], b = hex[(side + 1) % 6];

    p = (p + a + b) / 3;
    while (!queue.push(p) && running) {
      queue.push(p);
    }
  }
}

std::array<sf::Vector2i, 6> createHex(float scale = 1.f) {
  std::array<sf::Vector2i, 6> hex;

  float circumradius = 100 * scale;

  float xDist = circumradius * cos(30 * M_PI / 180);
  float yDist = circumradius * sin(30 * M_PI / 180);

  hex[0] = sf::Vector2i(0, int(circumradius));
  hex[1] = sf::Vector2i(int(xDist), int(yDist));
  hex[2] = sf::Vector2i(int(xDist), int(-yDist));
  hex[3] = sf::Vector2i(0, int(-circumradius));
  hex[4] = sf::Vector2i(int(-xDist), int(-yDist));
  hex[5] = sf::Vector2i(int(-xDist), int(yDist));

  return hex;
}

void drawHex(sf::RenderWindow& window, std::array<sf::Vector2i, 6>& hex) {
  sf::VertexArray hexagon(sf::LineStrip, 7);
  for (size_t i = 0; i < 7; i++) {
    hexagon[i].color = sf::Color::White;
    hexagon[i].position = sf::Vector2f(hex[i % 6]);
  }

  window.draw(hexagon);
}

void drawPoint(sf::RenderWindow& window, const sf::Vector2i& p) {
  sf::Vertex point(sf::Vector2f(p), sf::Color::White);
  window.draw(&point, 1, sf::Points);
}

int main() {
  sf::RenderWindow window(sf::VideoMode(800, 600), "Koch Snowflake");

  std::array<sf::Vector2i, 6> hex = createHex();
  sf::Vector2i initialPoint(0, 0);

  auto f = std::async(std::launch::async, fillQueue, hex, initialPoint);

  sf::View view(sf::Vector2f(0.f, 0.f), sf::Vector2f(200.f, 200.f));
  window.setView(view);

  window.clear(sf::Color::Black);
  drawHex(window, hex);
  window.display();

  sf::Clock clock;
  while (window.isOpen()) {
    sf::Event e;
    while (window.pollEvent(e)) {
      if (e.type == sf::Event::Closed || (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape)) {
        running = false;
        window.close();
      }
    }

    if (clock.getElapsedTime().asMilliseconds() > 1) {
      boost::optional<sf::Vector2i> p = queue.pop();
      if (p.has_value()) {
        drawPoint(window, p.value());
        window.display();
        clock.restart();
      }
    }
  }
}