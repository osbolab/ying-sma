#pragma once

#include <cstdint>

namespace sma
{
namespace detail
{
  template <typename T>
  struct arrcopy_w {
    T* arr;
    std::size_t len;
  };


  template <typename T>
  class array_buffer_view
  {
    using Myt = array_buffer_view<T>;

  public:
    using size_type = std::size_t;

    static Myt of(const T* c, size_type len);

    array_buffer_view(const Myt& r);
    array_buffer_view(Myt&& r);
    virtual ~array_buffer_view() {}

    Myt view() const;
    const T* cbuf() const { return c; }

    size_type limit() const { return lim; }
    size_type position() const { return pos; }
    size_type remaining() const { return lim - pos; }

    const T& operator[](size_type index) const { return c[index]; }
    const T& operator*() const { return c[pos]; }

    size_type get(T* dst, size_type len);

    template <typename A>
    Myt& operator>>(A& v);

    template <typename A>
    Myt& operator>>(const detail::arrcopy_w<A>& dst);

  protected:
    array_buffer_view(const T* c, size_type len);

    const T* c{nullptr};
    size_type lim{0};
    size_type pos{0};
  };
}
template <typename T>
detail::array_buffer_view<T> detail::array_buffer_view<T>::of(const T* c,
                                                              size_type len)
{
  return array_buffer_view(c, len);
}
template <typename T>
detail::array_buffer_view<T>::array_buffer_view(const T* c, size_type len)
  : c(c)
  , lim(len)
{
}
template <typename T>
detail::array_buffer_view<T>::array_buffer_view(const Myt& r)
  : c(r.c)
  , lim(r.lim)
  , pos(r.pos)
{
}
template <typename T>
detail::array_buffer_view<T>::array_buffer_view(Myt&& r)
  : c(r.c)
  , lim(r.lim)
  , pos(r.pos)
{
  r.c = nullptr;
  r.lim = 0;
  r.pos = 0;
}

template <typename T>
detail::array_buffer_view<T> detail::array_buffer_view<T>::view() const
{
  return array_buffer_view<T>(this);
}

template<typename T>
template <typename A>
detail::array_buffer_view<T>& detail::array_buffer_view<T>::operator>>(A& v)
{
  return *this;
}





template <typename T>
class array_buffer final : public detail::array_buffer_view<T>
{
  using Myt = array_buffer<T>;

public:
  using view = detail::array_buffer_view<T>;
  using size_type = typename view::size_type;

  static Myt wrap(T* a, size_type len);
  static Myt copy(T* a, size_type len);

  array_buffer(const Myt& r);
  array_buffer(Myt&& r);
  virtual ~array_buffer();

  inline Myt& flip();
  inline Myt& clear();

  T& operator[](size_type index) { return a[index]; }
  T& operator*() { return a[this->pos]; }

  Myt& put(const T* src, size_type len);

  template<typename A>
  Myt& operator<<(A v);

private:
  array_buffer(T* array, size_type len, bool owner = false);

  T* a{nullptr};
  bool owner{false};
  // The total capacity of the buffer, though the limit may change.
  size_type cap{0};
};

template <typename T>
array_buffer<T> array_buffer<T>::wrap(T* array, size_type len)
{
  return array_buffer<T>(array, len);
}

template <typename T>
array_buffer<T> array_buffer<T>::copy(T* array, size_type len)
{
  return array_buffer<T>(array, len, true);
}

template <typename T>
array_buffer<T>::array_buffer(T* array, size_type len, bool owner)
  : detail::array_buffer_view<T>(array, len)
  , a(array)
  , owner(owner)
  , cap(len)
{
}

template <typename T>
array_buffer<T>::array_buffer(const Myt& r)
  : detail::array_buffer_view<T>(r)
  , a(r.a)
  , owner(r.owner)
  , cap(r.cap)
{
  if (owner) {
    a = new T[cap];
    memcpy(a, r.a, cap * sizeof(T));
  }
}

template <typename T>
array_buffer<T>::array_buffer(Myt&& r)
  : detail::array_buffer_view<T>(r)
  , a(r.a)
  , owner(r.owner)
  , cap(r.cap)
{
  r.a = nullptr;
  r.owner = false;
  r.cap = 0;
}

template <typename T>
array_buffer<T>::~array_buffer()
{
  if (owner)
    delete[] a;
}


template <typename T>
array_buffer<T>& array_buffer<T>::flip()
{
  this->lim = this->pos;
  this->pos = 0;
}

template <typename T>
array_buffer<T>& array_buffer<T>::clear()
{
  this->lim = cap;
  this->pos = 0;
}



template <typename T>
static detail::arrcopy_w<T> arrcopy(T* arr, std::size_t len)
{
  return detail::arrcopy_w<T>{arr, len};
}
}
