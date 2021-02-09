#pragma once

#include <mutex>
#include <memory>

#define synchronized(monitor) \
  if (auto __lock = std::make_unique<std::lock_guard<std::mutex>>(monitor))