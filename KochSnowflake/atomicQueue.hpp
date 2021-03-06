#pragma once

#include <queue>
#include <mutex>
#include <boost/optional.hpp>

template<typename T>
class atomicQueue {
public:
  bool push(const T& val) {
    std::lock_guard<std::mutex> lock(mutex);
    if (queue.size() < 1000) {
      queue.push(val);
      return true;
    }

    return false;
  }

  boost::optional<T> pop() {
    std::lock_guard<std::mutex> lock(mutex);
    boost::optional<T> elem;
    if (!queue.empty()) {
      elem = queue.front();
      queue.pop();
    }
    
    return elem;
  }

  boost::optional<T> back() {
    std::lock_guard<std::mutex> lock(mutex);
    boost::optional<T> elem;
    if (!queue.empty()) {
      elem = queue.back();
    }

    return elem;
  }

  void clear() {
    std::lock_guard<std::mutex> lock(mutex);
    queue = {};
  }

private:
  std::queue<T> queue;
  mutable std::mutex mutex;
};