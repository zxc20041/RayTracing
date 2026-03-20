#ifndef INTERVAL_H
#define INTERVAL_H

#include <limits>

class interval {
  public:
    float min;
    float max;

    interval()
        : min(+std::numeric_limits<float>::infinity()),
          max(-std::numeric_limits<float>::infinity()) {}

    interval(float min, float max) : min(min), max(max) {}

    float size() const {
        return max - min;
    }

    bool contains(float x) const {
        return min <= x && x <= max;
    }

    bool surrounds(float x) const {
        return min < x && x < max;
    }

    float clamp(float x) const {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    static const interval empty;
    static const interval universe;
};

inline const interval interval::empty(+std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());
inline const interval interval::universe(-std::numeric_limits<float>::infinity(), +std::numeric_limits<float>::infinity());

#endif
