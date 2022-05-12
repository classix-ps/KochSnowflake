#pragma once

#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <boost/optional.hpp>

class SierpinskiPolygon {
public:
  SierpinskiPolygon();
  SierpinskiPolygon(const std::vector<sf::Vector2f>& vertices);

  void setPolygon(const std::vector<sf::Vector2f>& vertices);

  void reset();

  size_t getVertexCount();

  void addPoint(const sf::Vector2f& point);

  boost::optional<sf::Vector2f> lastPoint();

  void draw(sf::RenderWindow& window);

private:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar & polygon;
    ar & points;
  }

private:
  sf::VertexArray polygon;
  sf::VertexArray points;
};