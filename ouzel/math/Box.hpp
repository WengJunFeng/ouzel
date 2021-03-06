// Copyright 2015-2019 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_MATH_BOX_HPP
#define OUZEL_MATH_BOX_HPP

#include <cstdint>
#include <limits>
#include "math/Size.hpp"
#include "math/Vector.hpp"

namespace ouzel
{
    template<size_t N, class T> class Box final
    {
    public:
        Vector<N, T> min;
        Vector<N, T> max;

        Box()
        {
            for (T& v : min.v)
                v = std::numeric_limits<T>::max();
            for (T& v : max.v)
                v = std::numeric_limits<T>::lowest();
        }

        Box(const Vector<N, T>& initMin, const Vector<N, T>& initMax):
            min(initMin), max(initMax)
        {
        }

        template<size_t N2>
        explicit Box(const Box<N2, T>& box):
            min(Vector<N, T>(box.min)), max(Vector<N, T>(box.max))
        {
        }

        inline Vector<N, T> getCenter() const
        {
            return (min + max) / 2;
        }

        bool intersects(const Box& aabb) const
        {
            for (size_t i = 0; i < N; ++i)
                if (aabb.min.v[0] > max.v[0]) return false;
            for (size_t i = 0; i < N; ++i)
                if (aabb.max.v[0] < min.v[0]) return false;
            return true;
        }

        bool containsPoint(const Vector<N, T>& point) const
        {
            for (size_t i = 0; i < N; ++i)
                if (point.v[i] < min.v[i]) return false;
            for (size_t i = 0; i < N; ++i)
                if (point.v[i] > max.v[i]) return false;
            return true;
        }

        void merge(const Box& box)
        {
            for (size_t i = 0; i < N; ++i)
                min.v[i] = std::min(min.v[i], box.min.v[i]);
            for (size_t i = 0; i < N; ++i)
                max.v[i] = std::max(max.v[i], box.max.v[i]);
        }

        void reset()
        {
            for (T& v : min.v)
                v = std::numeric_limits<T>::max();
            for (T& v : max.v)
                v = std::numeric_limits<T>::lowest();
        }

        bool isEmpty() const
        {
            for (size_t i = 0; i < N; ++i)
                if (min.v[i] > max.v[i])
                    return true;
            return false;
        }

        void insertPoint(const Vector<N, T>& point)
        {
            for (size_t i = 0; i < N; ++i)
                if (point.v[i] < min.v[i]) min.v[i] = point.v[i];
            for (size_t i = 0; i < N; ++i)
                if (point.v[i] > max.v[i]) max.v[i] = point.v[i];
        }

        inline const Box operator+(const Vector<N, T>& v) const
        {
            Box result(*this);
            result.min += v;
            result.max += v;
            return result;
        }

        inline Box& operator+=(const Vector<N, T>& v)
        {
            min += v;
            max += v;
            return *this;
        }

        inline const Box operator-(const Vector<N, T>& v) const
        {
            Box result(*this);
            result.min -= v;
            result.max -= v;
            return result;
        }

        inline Box& operator-=(const Vector<N, T>& v)
        {
            min -= v;
            max -= v;
            return *this;
        }

        inline Size<N, T> getSize() const
        {
            return Size<N, T>(max - min);
        }
    };

    using Box2F = Box<2, float>;
    using Box3F = Box<3, float>;
}

#endif // OUZEL_MATH_BOX_HPP
