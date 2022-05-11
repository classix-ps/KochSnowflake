#include "sierpinskiPolygon.hpp"

SierpinskiPolygon::SierpinskiPolygon() {}

SierpinskiPolygon::SierpinskiPolygon(const std::vector<sf::Vector2f>& vertices) : polygon(sf::LineStrip, vertices.size() + 1) {
  size_t verticesCount = vertices.size();
  for (size_t i = 0; i < verticesCount + 1; i++) {
    polygon[i].color = sf::Color::White;
    polygon[i].position = sf::Vector2f(vertices[i % verticesCount]);
  }
}

void SierpinskiPolygon::setPolygon(const std::vector<sf::Vector2f>& vertices) {
  size_t verticesCount = vertices.size();
  polygon.setPrimitiveType(sf::LineStrip);
  polygon.resize(verticesCount + 1);
  for (size_t i = 0; i < verticesCount + 1; i++) {
    polygon[i].color = sf::Color::White;
    polygon[i].position = sf::Vector2f(vertices[i % verticesCount]);
  }
}

void SierpinskiPolygon::addPoint(const sf::Vector2f& point) {
  points.append(sf::Vertex(point));
}

void SierpinskiPolygon::draw(sf::RenderWindow& window) {
  window.draw(polygon);
  window.draw(points);
}