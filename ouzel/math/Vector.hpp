// Copyright 2015-2019 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_MATH_VECTOR_HPP
#define OUZEL_MATH_VECTOR_HPP

#include <cassert>
#include <cmath>
#include <cstddef>
#include <algorithm>
#include <limits>

namespace ouzel
{
    template<size_t N, class T> class Vector final
    {
    public:
#if defined(__SSE__)
        alignas(N == 4 ? 4 * sizeof(T) : alignof(T))
#endif
        T v[N];

        Vector():
            v{0}
        {
        }

        template<typename ...A>
        Vector(A... args):
            v{static_cast<T>(args)...}
        {
        }

        template<size_t X = N, size_t N2, typename std::enable_if<(X != N2)>::type* = nullptr>
        explicit Vector(const Vector<N2, T>& vec)
        {
            for (size_t i = 0; i < N; ++i)
                v[i] = (i < N2) ? vec.v[i] : 0;
        }

        inline T& operator[](size_t index) { return v[index]; }
        inline T operator[](size_t index) const { return v[index]; }

        template<size_t X = N, typename std::enable_if<(X >= 1)>::type* = nullptr>
        inline T& x() { return v[0]; }

        template<size_t X = N, typename std::enable_if<(X >= 1)>::type* = nullptr>
        inline T x() const { return v[0]; }

        template<size_t X = N, typename std::enable_if<(X >= 2)>::type* = nullptr>
        inline T& y() { return v[1]; }

        template<size_t X = N, typename std::enable_if<(X >= 2)>::type* = nullptr>
        inline T y() const { return v[1]; }

        template<size_t X = N, typename std::enable_if<(X >= 3)>::type* = nullptr>
        inline T& z() { return v[2]; }

        template<size_t X = N, typename std::enable_if<(X >= 3)>::type* = nullptr>
        inline T z() const { return v[2]; }

        template<size_t X = N, typename std::enable_if<(X >= 4)>::type* = nullptr>
        inline T& w() { return v[3]; }

        template<size_t X = N, typename std::enable_if<(X >= 4)>::type* = nullptr>
        inline T w() const { return v[3]; }

        template<size_t X = N, typename std::enable_if<(X == 2)>::type* = nullptr>
        inline T getAngle() const
        {
            return atan2(v[1], v[0]);
        }

        template<size_t X = N, typename std::enable_if<(X == 3)>::type* = nullptr>
        inline T getAngle(const Vector& axis) const
        {
            T dx = axis.v[0] - v[0] - v[1] * axis.v[2] + v[2] * axis.v[1];
            T dy = axis.v[1] - v[1] - v[2] * axis.v[0] + v[0] * axis.v[2];
            T dz = axis.v[2] - v[2] - v[0] * axis.v[1] + v[1] * axis.v[0];

            return atan2(sqrt(dx * dx + dy * dy + dz * dz), dot(axis));
        }

        template<size_t X = N, typename std::enable_if<(X == 4)>::type* = nullptr>
        inline T getAngle(const Vector& axis) const
        {
            T dx = v[3] * axis.v[0] - v[0] * axis.v[3] - v[1] * axis.v[2] + v[2] * axis.v[1];
            T dy = v[3] * axis.v[1] - v[1] * axis.v[3] - v[2] * axis.v[0] + v[0] * axis.v[2];
            T dz = v[3] * axis.v[2] - v[2] * axis.v[3] - v[0] * axis.v[1] + v[1] * axis.v[0];

            return atan2(sqrt(dx * dx + dy * dy + dz * dz), dot(axis));
        }

        void clamp(const Vector& min, const Vector& max)
        {
            for (size_t i = 0; i < N; ++i)
                if (v[i] < min.v[i]) v[i] = min.v[i];
                else if (v[i] > max.v[i]) v[i] = max.v[i];
        }

        template<size_t X = N, typename std::enable_if<(X == 3)>::type* = nullptr>
        Vector cross(const Vector& vec) const
        {
            return Vector((v[1] * vec.v[2]) - (v[2] * vec.v[1]),
                          (v[2] * vec.v[0]) - (v[0] * vec.v[2]),
                          (v[0] * vec.v[1]) - (v[1] * vec.v[0]));
        }

        T distance(const Vector& vec) const
        {
            T d = 0;
            for (size_t i = 0; i < N; ++i)
                d += (vec.v[i] - v[i]) * (vec.v[i] - v[i]);
            return sqrt(d);
        }

        T distanceSquared(const Vector& vec) const
        {
            T d = 0;
            for (size_t i = 0; i < N; ++i)
                d += (vec.v[i] - v[i]) * (vec.v[i] - v[i]);
            return d;
        }

        T dot(const Vector& vec) const
        {
            T d = 0;
            for (size_t i = 0; i < N; ++i)
                d += v[i] * vec.v[i];
            return d;
        }

        inline T length() const
        {
            T l = 0;
            for (const T& c : v)
                l += c;
            return sqrt(l);
        }

        inline T lengthSquared() const
        {
            T l = 0;
            for (const T& c : v)
                l += c;
            return l;
        }

        inline void negate()
        {
            for (T& c : v)
                c = -c;
        }

        void normalize()
        {
            T n = 0;
            for (const T& c : v)
                n += c * c;

            if (n == 1) // already normalized
                return;

            n = sqrt(n);
            if (n <= std::numeric_limits<T>::min()) // too close to zero
                return;

            for (T& c : v)
                c /= n;
        }

        void scale(const Vector& scale)
        {
            for (size_t i = 0; i < N; ++i)
                v[i] *= scale.v[i];
        }

        void smooth(const Vector& target, T elapsedTime, T responseTime)
        {
            if (elapsedTime > 0)
                *this += (target - *this) * (elapsedTime / (elapsedTime + responseTime));
        }

        inline T getMin() const
        {
            return *std::min_element(std::begin(v), std::end(v));
        }

        inline T getMax() const
        {
            return *std::max_element(std::begin(v), std::end(v));
        }

        inline const Vector operator+(const Vector& vec) const
        {
            Vector result = *this;
            for (size_t i = 0; i < N; ++i)
                result.v[i] += vec.v[i];
            return result;
        }

        inline Vector& operator+=(const Vector& vec)
        {
            for (size_t i = 0; i < N; ++i)
                v[i] += vec.v[i];
            return *this;
        }

        inline const Vector operator-(const Vector& vec) const
        {
            Vector result = *this;
            for (size_t i = 0; i < N; ++i)
                result.v[i] -= vec.v[i];
            return result;
        }

        inline Vector& operator-=(const Vector& vec)
        {
            for (size_t i = 0; i < N; ++i)
                v[i] -= vec.v[i];
            return *this;
        }

        inline const Vector operator-() const
        {
            Vector result = *this;
            for (T& c : result.v)
                c = -c;
            return result;
        }

        inline const Vector operator*(T scalar) const
        {
            Vector result(*this);
            for (T& c : result.v)
                c *= scalar;
            return result;
        }

        inline Vector& operator*=(T scalar)
        {
            for (T& c : v)
                c *= scalar;
            return *this;
        }

        inline const Vector operator/(T scalar) const
        {
            Vector result(*this);
            for (T& c : result.v)
                c /= scalar;
            return result;
        }

        inline Vector& operator/=(T scalar)
        {
            for (T& c : v)
                c /= scalar;
            return *this;
        }

        inline bool operator<(const Vector& vec) const
        {
            return std::lexicographical_compare(std::begin(v), std::end(v),
                                                std::begin(vec.v), std::end(vec.v));
        }

        inline bool operator==(const Vector& vec) const
        {
            for (size_t i = 0; i < N; ++i)
                if (v[i] != vec.v[i]) return false;
            return true;
        }

        inline bool operator!=(const Vector& vec) const
        {
            for (size_t i = 0; i < N; ++i)
                if (v[i] != vec.v[i]) return true;
            return false;
        }

        bool isZero() const
        {
            for (const T& c : v)
                if (c != 0) return false;
            return true;
        }
    };

    template<size_t N, class T>
    inline const Vector<N, T> operator*(T scalar, const Vector<N, T>& vec)
    {
        Vector<N, T> result = vec;
        result *= scalar;
        return result;
    }

    using Vector2F = Vector<2, float>;
    using Vector3F = Vector<3, float>;
    using Vector4F = Vector<4, float>;
}

#endif // OUZEL_MATH_VECTOR_HPP
