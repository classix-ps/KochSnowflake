#include "windowManager.hpp"

#include <SFML/Graphics.hpp>

#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <math.h>
#include <random>
#include <algorithm>

WindowManager::WindowManager() : window(sf::VideoMode(800, 600), "Koch Snowflake"), initialPoint(0, 0) {
  vertices = createVertices();
  std::string path = saveDir + std::to_string(vertices.size()) + "gon.dat";
  std::ifstream ifs(path);
  if (ifs.good()) {
    boost::archive::text_iarchive ia(ifs);
    ia >> sierpinskiPolygon;
    ifs.close();
  }
  else {
    sierpinskiPolygon.setPolygon(vertices);
  }

  view = window.getDefaultView();
  view.setCenter(sf::Vector2f(0.f, 0.f));
  view.zoom(zoom);
  window.setView(view);
}

void WindowManager::run() {
  while (window.isOpen()) {
    handleEvents();

    if (running && clock.getElapsedTime().asMicroseconds() > drawInterval) {
      boost::optional<sf::Vector2f> p = queue.pop();
      if (p.has_value()) {
        sierpinskiPolygon.addPoint(p.value());
      }

      clock.restart();
    }

    window.clear();
    sierpinskiPolygon.draw(window);
    window.display();
  }

  if (calculationThread.valid()) {
    calculationThread.wait();
  }
}

void WindowManager::handleEvents() {
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
        oldMousePos = sf::Vector2f(sf::Mouse::getPosition());
        state = State::Move;
      }
      else if (e.type == sf::Event::KeyPressed) {
        switch (e.key.code) {
        case sf::Keyboard::C:
          view.setCenter(sf::Vector2f(0.f, 0.f));
          window.setView(view);
          break;

        case sf::Keyboard::Space:
          if (running) {
            running = false;
            calculationThread.wait();
          }
          else {
            running = true;
            calculationThread = std::async(std::launch::async, &WindowManager::fillQueue, this);
            clock.restart();
          }
          break;

        case sf::Keyboard::S:
        {
          std::string path = saveDir + std::to_string(sierpinskiPolygon.getVertexCount()) + "gon.dat";
          std::ofstream ofs(path);
          boost::archive::text_oarchive oa(ofs);
          oa << sierpinskiPolygon;
          ofs.close();
          break;
        }

        case sf::Keyboard::R:
          running = false;
          if (calculationThread.valid()) {
            calculationThread.wait();
          }

          sierpinskiPolygon.reset();
          sierpinskiPolygon.setPolygon(vertices);

          queue.clear();
          break;

        case sf::Keyboard::Right:
        {
          size_t vertexCount = sierpinskiPolygon.getVertexCount();
          if (vertexCount > 19) {
            break;
          }

          running = false;
          if (calculationThread.valid()) {
            calculationThread.wait();
          }

          sierpinskiPolygon.reset();
          vertices = createVertices(vertexCount + 1);
          std::string path = saveDir + std::to_string(vertexCount + 1) + "gon.dat";
          std::ifstream ifs(path);
          if (ifs.good()) {
            boost::archive::text_iarchive ia(ifs);
            ia >> sierpinskiPolygon;
            ifs.close();
          }
          else {
            sierpinskiPolygon.setPolygon(vertices);
          }

          queue.clear();
          break;
        }

        case sf::Keyboard::Left:
        {
          size_t vertexCount = sierpinskiPolygon.getVertexCount();
          if (vertexCount < 4) {
            break;
          }

          running = false;
          if (calculationThread.valid()) {
            calculationThread.wait();
          }

          sierpinskiPolygon.reset();
          vertices = createVertices(vertexCount - 1);
          std::string path = saveDir + std::to_string(vertexCount - 1) + "gon.dat";
          std::ifstream ifs(path);
          if (ifs.good()) {
            boost::archive::text_iarchive ia(ifs);
            ia >> sierpinskiPolygon;
            ifs.close();
          }
          else {
            sierpinskiPolygon.setPolygon(vertices);
          }

          queue.clear();
          break;
        }
        }
      }
      break;

    case State::Move:
      if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left) {
        state = State::View;
      }
      else {
        sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition());
        sf::Vector2f deltaPos = oldMousePos - mousePos;
        deltaPos.x *= zoom;
        deltaPos.y *= zoom;
        view.move(deltaPos);
        window.setView(view);
        oldMousePos = mousePos;
      }
      break;
    }
  }
}

/* Fills the queue with points.
 */
void WindowManager::fillQueue() {
  size_t vertexCount = vertices.size();

  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_int_distribution<size_t> uid(0, vertexCount - 1);

  sf::Vector2f p;
  if (boost::optional<sf::Vector2f> lastQueue = queue.back()) {
    p = lastQueue.value();
  }
  else if (boost::optional<sf::Vector2f> lastSierpinskiPolygon = sierpinskiPolygon.lastPoint()) {
    p = lastSierpinskiPolygon.value();
  }
  while (running) {
    size_t side = uid(rng);
    sf::Vector2f a = vertices[side], b = vertices[(side + 1) % vertexCount];

    p = (p + a + b) / 3.f;
    while (!queue.push(p) && running) {}
  }
}

std::vector<sf::Vector2f> WindowManager::createVertices(size_t vertexCount) {
  std::vector<sf::Vector2f> vertices;

  float circumradius = 100.f;
  float angleDelta = 360.f / vertexCount;
  float angle = 90 + angleDelta / 2;

  for (size_t i = 0; i < vertexCount; i++) {
    vertices.push_back(sf::Vector2f(circumradius * cos(angle * M_PI / 180), circumradius * sin(angle * M_PI / 180)));
    angle += angleDelta;
  }

  return vertices;
}