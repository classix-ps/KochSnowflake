#pragma once

#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

class SierpinskiPolygon {
public:
  SierpinskiPolygon();
  SierpinskiPolygon(const std::vector<sf::Vector2f>& vertices);

  void setPolygon(const std::vector<sf::Vector2f>& vertices);

  void addPoint(const sf::Vector2f& point);

  void draw(sf::RenderWindow& window);

private:
  sf::VertexArray polygon;
  sf::VertexArray points;
};