#include <SFML/Graphics.hpp>

#include "atomicQueue.hpp"

#include <math.h>
#include <future>
#include <random>

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

std::vector<sf::Vector2f> createVertices(size_t verticesCount = 6, float scale = 1.f) {
  std::vector<sf::Vector2f> vertices;

  float circumradius = 100 * scale;
  float angleDelta = 360 / verticesCount;
  float angle = 90 + angleDelta / 2;

  for (size_t i = 0; i < verticesCount; i++) {
    vertices.push_back(sf::Vector2f(circumradius * cos(angle * M_PI / 180), circumradius * sin(angle * M_PI / 180)));
    angle += angleDelta;
  }

  return vertices;
}

sf::VertexArray createPolygon(const std::vector<sf::Vector2f>& vertices) {
  size_t verticesCount = vertices.size();
  sf::VertexArray polygon(sf::LineStrip, verticesCount + 1);
  for (size_t i = 0; i < verticesCount + 1; i++) {
    polygon[i].color = sf::Color::White;
    polygon[i].position = sf::Vector2f(vertices[i % verticesCount]);
  }

  return polygon;
}

int main() {
  sf::RenderWindow window(sf::VideoMode(800, 600), "Koch Snowflake");

  std::vector<sf::Vector2f> vertices = createVertices(6);
  sf::Vector2f initialPoint(0, 0);

  auto f = std::async(std::launch::async, fillQueue, vertices, initialPoint);

  sf::View view(sf::Vector2f(0.f, 0.f), sf::Vector2f(200.f, 200.f));
  window.setView(view);

  window.clear(sf::Color::Black);

  sf::VertexArray polygon = createPolygon(vertices);
  sf::VertexArray points;

  sf::Clock clock;
  while (window.isOpen()) {
    sf::Event e;
    while (window.pollEvent(e)) {
      if (e.type == sf::Event::Closed || (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape)) {
        running = false;
        window.close();
      }
    }

    if (clock.getElapsedTime().asMicroseconds() > 1) {
      boost::optional<sf::Vector2f> p = queue.pop();
      if (p.has_value()) {
        points.append(sf::Vertex(sf::Vector2f(p.value()), sf::Color::White));
      }
    }

    window.clear();
    window.draw(polygon);
    window.draw(points);
    window.display();
  }
}