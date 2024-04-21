#pragma once
#include <new>
#include <cstddef>
#include "esp_heap_caps.h"

template <typename T>
class PSRAMAllocator 
{
  public:
  typedef T value_type;

  PSRAMAllocator() = default;
  template <class U> constexpr PSRAMAllocator(const PSRAMAllocator<U>&) noexcept {}

  T* allocate(std::size_t n) 
  {
    return static_cast<T*>(heap_caps_malloc(n*sizeof(T), MALLOC_CAP_SPIRAM));
  }

  void deallocate(T* p, std::size_t) noexcept { heap_caps_free(p); }
};

template <class T, class U>
bool operator==(const PSRAMAllocator<T>&, const PSRAMAllocator<U>&) { return true; }

template <class T, class U>
bool operator!=(const PSRAMAllocator<T>&, const PSRAMAllocator<U>&) { return false; }