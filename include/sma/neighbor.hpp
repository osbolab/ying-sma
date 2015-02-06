#pragma once

#include <sma/chrono.hpp>
#include <sma/util/vec2d.hpp>
#include <sma/util/circularbuffer.hpp>


namespace sma
{
struct Neighbor {
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

  Neighbor(Vec2d position)
    : first_seen(clock::now())
  {
    saw(position);
  }

  bool is_new() const { return first_seen == last_seen; }

  template <typename D>
  bool older_than(D const& age) const
  {
    return std::chrono::duration_cast<D>(clock::now() - last_seen) >= age;
  }

  void saw(Vec2d position)
  {
    auto k = positions.push_back(position);
    if (k > 0)
      means.push_back(compute_mean(k));
    velocity = compute_velocity();

    last_seen = clock::now();
    times_pinged = 0;
  }

  CircularBuffer<Vec2d, 4> positions;
  Vec2d velocity;
  time_point first_seen;
  time_point last_seen;
  unsigned int times_pinged;

private:
  CircularBuffer<Vec2d, 4> means;

  Vec2d compute_mean(std::size_t k) const
  {
    assert(k > 0);
    return means[k - 1] + (positions[k] - means[k - 1]) / double(k);
  }

  Vec2d compute_velocity() const
  {
    if (positions.size() < 4)
      return Vec2d(0.0, 0.0);

    Vec2d deviation;
    for (std::size_t k = 1; k < positions.size(); ++k)
      deviation += (positions[k] - means[k - 1]) / (positions[k] - means[k]);

    auto const k = positions.size() - 1;
    return positions[k] / Vec2d::exp(Vec2d::divide(4.0, means[k]) * deviation);
  }
};
}
