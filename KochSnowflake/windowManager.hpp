#pragma once

#include <SFML/System/Vector2.hpp>

#include "atomicQueue.hpp"
#include "sierpinskiPolygon.hpp"

#include <future>

template<typename T>
class atomicQueue;

class WindowManager {
public:
  WindowManager();

  void run();

private:
  enum class State { View, Move };

  void fillQueue();

  std::vector<sf::Vector2f> createVertices(size_t vertexCount = 6);

  void handleEvents();

  sf::RenderWindow window;
  sf::View view;
  float zoom = 0.4f;
  float maxZoom = 1.f;
  float minZoom = 0.001f;

  std::atomic_bool running = false;
  atomicQueue<sf::Vector2f> queue;
  std::future<void> calculationThread;

  sf::Vector2f initialPoint;
  std::vector<sf::Vector2f> vertices;

  SierpinskiPolygon sierpinskiPolygon;

  std::string saveDir = "save/";

  sf::Clock clock;
  sf::Int64 drawInterval = 1;
  sf::Vector2f oldMousePos;
  
  State state = State::View;
};