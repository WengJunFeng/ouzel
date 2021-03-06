// Copyright 2015-2019 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_MATH_MATRIX_HPP
#define OUZEL_MATH_MATRIX_HPP

#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include "math/ConvexVolume.hpp"
#include "math/Plane.hpp"
#include "math/Quaternion.hpp"
#include "math/Vector.hpp"

namespace ouzel
{
    template<size_t C, size_t R, class T> class Matrix final
    {
    public:
#if defined(__SSE__)
        alignas((C == 4 && R == 4) ? 4 * sizeof(T) : alignof(T))
#endif
        T m[C * R];

        Matrix():
            m{0}
        {
        }

        template<typename ...A>
        Matrix(A... args):
            m{static_cast<T>(args)...}
        {
        }

        explicit Matrix(const T* array)
        {
            for (size_t i = 0; i < C * R; ++i)
                m[i] = array[i];
        }

        T& operator[](size_t index) { return m[index]; }
        T operator[](size_t index) const { return m[index]; }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        static inline constexpr Matrix identity()
        {
            return Matrix(1, 0, 0, 0,
                          0, 1, 0, 0,
                          0, 0, 1, 0,
                          0, 0, 0, 1);
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void setLookAt(const Vector<3, T>& eyePosition,
                       const Vector<3, T>& targetPosition,
                       const Vector<3, T>& up)
        {
            setLookAt(eyePosition.v[0], eyePosition.v[1], eyePosition.v[2],
                      targetPosition.v[0], targetPosition.v[1], targetPosition.v[2],
                      up.v[0], up.v[1], up.v[2]);
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void setLookAt(T eyePositionX, T eyePositionY, T eyePositionZ,
                       T targetPositionX, T targetPositionY, T targetPositionZ,
                       T upX, T upY, T upZ)
        {
            Vector<3, T> eye(eyePositionX, eyePositionY, eyePositionZ);
            Vector<3, T> target(targetPositionX, targetPositionY, targetPositionZ);
            Vector<3, T> up(upX, upY, upZ);
            up.normalize();

            Vector<3, T> zaxis = target - eye;
            zaxis.normalize();

            Vector<3, T> xaxis = up.cross(zaxis);
            xaxis.normalize();

            Vector<3, T> yaxis = zaxis.cross(xaxis);
            yaxis.normalize();

            m[0] = xaxis.v[0];
            m[1] = yaxis.v[0];
            m[2] = zaxis.v[0];
            m[3] = 0;

            m[4] = xaxis.v[1];
            m[5] = yaxis.v[1];
            m[6] = zaxis.v[1];
            m[7] = 0;

            m[8] = xaxis.v[2];
            m[9] = yaxis.v[2];
            m[10] = zaxis.v[2];
            m[11] = 0;

            m[12] = xaxis.dot(-eye);
            m[13] = yaxis.dot(-eye);
            m[14] = zaxis.dot(-eye);
            m[15] = 1;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void setPerspective(T fieldOfView, T aspectRatio, T zNearPlane, T zFarPlane)
        {
            assert(zFarPlane != zNearPlane);

            T theta = fieldOfView / 2;
            if (fabs(fmod(theta, pi<T>() / 2)) <= std::numeric_limits<T>::min())
                return;

            T divisor = tan(theta);
            assert(divisor);
            T factor = 1 / divisor;

            setZero();

            assert(aspectRatio);
            m[0] = (1 / aspectRatio) * factor;
            m[5] = factor;
            m[10] = zFarPlane / (zFarPlane - zNearPlane);
            m[11] = 1;
            m[14] = -zNearPlane * zFarPlane / (zFarPlane - zNearPlane);
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void setOrthographicFromSize(T width, T height, T zNearPlane, T zFarPlane)
        {
            T halfWidth = width / 2;
            T halfHeight = height / 2;
            setOrthographicOffCenter(-halfWidth, halfWidth,
                                     -halfHeight, halfHeight,
                                     zNearPlane, zFarPlane);
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void setOrthographicOffCenter(T left, T right, T bottom, T top,
                                      T zNearPlane, T zFarPlane)
        {
            assert(right != left);
            assert(top != bottom);
            assert(zFarPlane != zNearPlane);

            setZero();

            m[0] = 2 / (right - left);
            m[5] = 2 / (top - bottom);
            m[10] = 1 / (zFarPlane - zNearPlane);
            m[12] = (left + right) / (left - right);
            m[13] = (bottom + top) / (bottom - top);
            m[14] = zNearPlane / (zNearPlane - zFarPlane);
            m[15] = 1;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 3 && Y == 3)>::type* = nullptr>
        void setScale(const Vector<2, T>& scale)
        {
            setIdentity();

            m[0] = scale.v[0];
            m[4] = scale.v[1];
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void setScale(const Vector<3, T>& scale)
        {
            setIdentity();

            m[0] = scale.v[0];
            m[5] = scale.v[1];
            m[10] = scale.v[2];
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 3 && Y == 3)>::type* = nullptr>
        void setScale(T x, T y)
        {
            setIdentity();

            m[0] = x;
            m[4] = y;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void setScale(T x, T y, T z)
        {
            setIdentity();

            m[0] = x;
            m[5] = y;
            m[10] = z;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 3 && Y == 3)>::type* = nullptr>
        void setRotation(T angle)
        {
            setIdentity();

            T c = cos(angle);
            T s = sin(angle);

            m[0] = c;
            m[3] = -s;
            m[1] = s;
            m[4] = c;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void setRotation(const Vector<3, T>& axis, T angle)
        {
            T x = axis.v[0];
            T y = axis.v[1];
            T z = axis.v[2];

            // Make sure the input axis is normalized
            T n = x * x + y * y + z * z;
            if (n != 1)
            {
                // Not normalized
                n = sqrt(n);
                // Prevent divide too close to zero
                if (n > std::numeric_limits<T>::min())
                {
                    n = 1 / n;
                    x *= n;
                    y *= n;
                    z *= n;
                }
            }

            T c = cos(angle);
            T s = sin(angle);

            T t = 1 - c;
            T tx = t * x;
            T ty = t * y;
            T tz = t * z;
            T txy = tx * y;
            T txz = tx * z;
            T tyz = ty * z;
            T sx = s * x;
            T sy = s * y;
            T sz = s * z;

            m[0] = c + tx * x;
            m[4] = txy - sz;
            m[8] = txz + sy;
            m[12] = 0;

            m[1] = txy + sz;
            m[5] = c + ty * y;
            m[9] = tyz - sx;
            m[13] = 0;

            m[2] = txz - sy;
            m[6] = tyz + sx;
            m[10] = c + tz * z;
            m[14] = 0;

            m[3] = 0;
            m[7] = 0;
            m[11] = 0;
            m[15] = 1;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void setRotation(const Quaternion<T>& rotation)
        {
            T wx = rotation.v[3] * rotation.v[0];
            T wy = rotation.v[3] * rotation.v[1];
            T wz = rotation.v[3] * rotation.v[2];

            T xx = rotation.v[0] * rotation.v[0];
            T xy = rotation.v[0] * rotation.v[1];
            T xz = rotation.v[0] * rotation.v[2];

            T yy = rotation.v[1] * rotation.v[1];
            T yz = rotation.v[1] * rotation.v[2];

            T zz = rotation.v[2] * rotation.v[2];

            m[0] = 1 - 2 * (yy + zz);
            m[4] = 2 * (xy - wz);
            m[8] = 2 * (xz + wy);
            m[12] = 0;

            m[1] = 2 * (xy + wz);
            m[5] = 1 - 2 * (xx + zz);
            m[9] = 2 * (yz - wx);
            m[13] = 0;

            m[2] = 2 * (xz - wy);
            m[6] = 2 * (yz + wx);
            m[10] = 1 - 2 * (xx + yy);
            m[14] = 0;

            m[3] = 0;
            m[7] = 0;
            m[11] = 0;
            m[15] = 1;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void setRotationX(T angle)
        {
            setIdentity();

            T c = cos(angle);
            T s = sin(angle);

            m[5] = c;
            m[9] = -s;
            m[6] = s;
            m[10] = c;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void setRotationY(T angle)
        {
            setIdentity();

            T c = cos(angle);
            T s = sin(angle);

            m[0] = c;
            m[8] = s;
            m[2] = -s;
            m[10] = c;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void setRotationZ(T angle)
        {
            setIdentity();

            T c = cos(angle);
            T s = sin(angle);

            m[0] = c;
            m[4] = -s;
            m[1] = s;
            m[5] = c;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 3 && Y == 3)>::type* = nullptr>
        void setTranslation(const Vector<3, T>& translation)
        {
            setIdentity();

            m[6] = translation.v[0];
            m[7] = translation.v[1];
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void setTranslation(const Vector<3, T>& translation)
        {
            setIdentity();

            m[12] = translation.v[0];
            m[13] = translation.v[1];
            m[14] = translation.v[2];
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 3 && Y == 3)>::type* = nullptr>
        void setTranslation(T x, T y)
        {
            setIdentity();

            m[6] = x;
            m[7] = y;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void setTranslation(T x, T y, T z)
        {
            setIdentity();

            m[12] = x;
            m[13] = y;
            m[14] = z;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        Plane<T> getFrustumLeftPlane() const
        {
            return Plane<T>::makeFrustumPlane(m[3] + m[0], m[7] + m[4], m[11] + m[8], m[15] + m[12]);
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        inline Plane<T> getFrustumRightPlane() const
        {
            return Plane<T>::makeFrustumPlane(m[3] - m[0], m[7] - m[4], m[11] - m[8], m[15] - m[12]);
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        inline Plane<T> getFrustumBottomPlane() const
        {
            return Plane<T>::makeFrustumPlane(m[3] + m[1], m[7] + m[5], m[11] + m[9], m[15] + m[13]);
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        inline Plane<T> getFrustumTopPlane() const
        {
            return Plane<T>::makeFrustumPlane(m[3] - m[1], m[7] - m[5], m[11] - m[9], m[15] - m[13]);
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        inline Plane<T> getFrustumNearPlane() const
        {
            return Plane<T>::makeFrustumPlane(m[3] + m[2], m[7] + m[6], m[11] + m[10], m[15] + m[14]);
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        inline Plane<T> getFrustumFarPlane() const
        {
            return Plane<T>::makeFrustumPlane(m[3] - m[2], m[7] - m[6], m[11] - m[10], m[15] - m[14]);
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        ConvexVolume<T> getFrustum() const
        {
            ConvexVolume<T> frustum;
            frustum.planes.push_back(getFrustumLeftPlane());
            frustum.planes.push_back(getFrustumRightPlane());
            frustum.planes.push_back(getFrustumBottomPlane());
            frustum.planes.push_back(getFrustumTopPlane());
            frustum.planes.push_back(getFrustumNearPlane());
            frustum.planes.push_back(getFrustumFarPlane());
            return frustum;
        }

        void add(T scalar)
        {
            add(scalar, *this);
        }

        void add(T scalar, Matrix& dst);

        void add(const Matrix& matrix)
        {
            add(*this, matrix, *this);
        }

        static void add(const Matrix& m1, const Matrix& m2, Matrix& dst);

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 1 && Y == 1)>::type* = nullptr>
        T determinant() const
        {
            return m[0];
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 2 && Y == 2)>::type* = nullptr>
        T determinant() const
        {
            return m[0] * m[3] - m[1] * m[2];
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 3 && Y == 3)>::type* = nullptr>
        T determinant() const
        {
            T a0 = m[0] * (m[4] * m[8] - m[5] * m[7]);
            T a1 = m[1] * (m[3] * m[8] - m[5] * m[6]);
            T a2 = m[2] * (m[3] * m[7] - m[4] * m[6]);
            return a0 - a1 + a2;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        T determinant() const
        {
            T a0 = m[0] * m[5] - m[1] * m[4];
            T a1 = m[0] * m[6] - m[2] * m[4];
            T a2 = m[0] * m[7] - m[3] * m[4];
            T a3 = m[1] * m[6] - m[2] * m[5];
            T a4 = m[1] * m[7] - m[3] * m[5];
            T a5 = m[2] * m[7] - m[3] * m[6];
            T b0 = m[8] * m[13] - m[9] * m[12];
            T b1 = m[8] * m[14] - m[10] * m[12];
            T b2 = m[8] * m[15] - m[11] * m[12];
            T b3 = m[9] * m[14] - m[10] * m[13];
            T b4 = m[9] * m[15] - m[11] * m[13];
            T b5 = m[10] * m[15] - m[11] * m[14];
            return a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        inline void getUpVector(Vector<3, T>& dst) const
        {
            dst.v[0] = m[4];
            dst.v[1] = m[5];
            dst.v[2] = m[6];
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        inline void getDownVector(Vector<3, T>& dst) const
        {
            dst.v[0] = -m[4];
            dst.v[1] = -m[5];
            dst.v[2] = -m[6];
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        inline void getLeftVector(Vector<3, T>& dst) const
        {
            dst.v[0] = -m[0];
            dst.v[1] = -m[1];
            dst.v[2] = -m[2];
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        inline void getRightVector(Vector<3, T>& dst) const
        {
            dst.v[0] = m[0];
            dst.v[1] = m[1];
            dst.v[2] = m[2];
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        inline void getForwardVector(Vector<3, T>& dst) const
        {
            dst.v[0] = -m[8];
            dst.v[1] = -m[9];
            dst.v[2] = -m[10];
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        inline void getBackVector(Vector<3, T>& dst) const
        {
            dst.v[0] = m[8];
            dst.v[1] = m[9];
            dst.v[2] = m[10];
        }

        void invert()
        {
            invert(*this);
        }

        void invert(Matrix& dst) const;

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == Y)>::type* = nullptr>
        inline bool isIdentity() const
        {
            for (size_t r = 0; r < R; ++r)
                for (size_t c = 0; c < C; ++c)
                    if (m[r * C + c] != (r == c ? 1 : 0))
                        return false;
            return true;
        }

        void multiply(T scalar)
        {
            multiply(scalar, *this);
        }

        void multiply(T scalar, Matrix& dst) const
        {
            multiply(*this, scalar, dst);
        }

        static void multiply(const Matrix& m, T scalar, Matrix& dst);

        void multiply(const Matrix& matrix)
        {
            multiply(*this, matrix, *this);
        }

        static void multiply(const Matrix& m1, const Matrix& m2, Matrix& dst);

        void negate()
        {
            negate(*this);
        }

        void negate(Matrix& dst) const;

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == Y)>::type* = nullptr>
        inline void setIdentity()
        {
            for (size_t r = 0; r < R; ++r)
                for (size_t c = 0; c < C; ++c)
                    m[r * C + c] = static_cast<T>(r == c ? 1 : 0);
        }

        inline void setZero()
        {
            for (size_t i = 0; i < C * R; ++i)
                m[i] = 0;
        }

        void subtract(const Matrix& matrix)
        {
            subtract(*this, matrix, *this);
        }

        static void subtract(const Matrix& m1, const Matrix& m2, Matrix& dst);

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void transformPoint(Vector<3, T>& point) const
        {
            transformVector(point.v[0], point.v[1], point.v[2], 1, point);
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void transformPoint(const Vector<3, T>& point, Vector<3, T>& dst) const
        {
            transformVector(point.v[0], point.v[1], point.v[2], 1, dst);
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void transformVector(Vector<3, T>& v) const
        {
            Vector<4, T> t;
            transformVector(Vector<4, T>(v.v[0], v.v[1], v.v[2], static_cast<T>(0)), t);
            v = Vector<3, T>(t.v[0], t.v[1], t.v[2]);
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void transformVector(const Vector<3, T>& v, Vector<3, T>& dst) const
        {
            transformVector(v.v[0], v.v[1], v.v[2], 0, dst);
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void transformVector(T x, T y, T z, T w, Vector<3, T>& dst) const
        {
            Vector<4, T> t;
            transformVector(Vector<4, T>(x, y, z, w), t);
            dst = Vector<3, T>(t.v[0], t.v[1], t.v[2]);
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        void transformVector(Vector<4, T>& v) const
        {
            transformVector(v, v);
        }

        void transformVector(const Vector<4, T>& v, Vector<4, T>& dst) const;

        void transpose()
        {
            transpose(*this);
        }

        void transpose(Matrix& dst) const;

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 3 && Y == 3)>::type* = nullptr>
        Vector<2, T> getTranslation() const
        {
            return Vector<2, T>(m[6], m[7]);
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        Vector<3, T> getTranslation() const
        {
            return Vector<3, T>(m[12], m[13], m[14]);
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 3 && Y == 3)>::type* = nullptr>
        Vector<2, T> getScale() const
        {
            Vector<2, T> scale;
            scale.v[0] = Vector<2, T>(m[0], m[1]).length();
            scale.v[1] = Vector<2, T>(m[3], m[4]).length();

            return scale;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        Vector<3, T> getScale() const
        {
            Vector<3, T> scale;
            scale.v[0] = Vector<3, T>(m[0], m[1], m[2]).length();
            scale.v[1] = Vector<3, T>(m[4], m[5], m[6]).length();
            scale.v[2] = Vector<3, T>(m[8], m[9], m[10]).length();

            return scale;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        Quaternion<T> getRotation() const
        {
            Vector<3, T> scale = getScale();

            T m11 = m[0] / scale.v[0];
            T m21 = m[1] / scale.v[0];
            T m31 = m[2] / scale.v[0];

            T m12 = m[4] / scale.v[1];
            T m22 = m[5] / scale.v[1];
            T m32 = m[6] / scale.v[1];

            T m13 = m[8] / scale.v[2];
            T m23 = m[9] / scale.v[2];
            T m33 = m[10] / scale.v[2];

            Quaternion<T> result;
            result.v[0] = sqrt(std::max(static_cast<T>(0), 1 + m11 - m22 - m33)) / 2;
            result.v[1] = sqrt(std::max(static_cast<T>(0), 1 - m11 + m22 - m33)) / 2;
            result.v[2] = sqrt(std::max(static_cast<T>(0), 1 - m11 - m22 + m33)) / 2;
            result.v[3] = sqrt(std::max(static_cast<T>(0), 1 + m11 + m22 + m33)) / 2;

            result.v[0] *= sgn(result.v[0] * (m32 - m23));
            result.v[1] *= sgn(result.v[1] * (m13 - m31));
            result.v[2] *= sgn(result.v[2] * (m21 - m12));

            result.normalize();

            return result;
        }

        inline const Matrix operator+(const Matrix& matrix) const
        {
            Matrix result(*this);
            add(result, matrix, result);
            return result;
        }

        inline Matrix& operator+=(const Matrix& matrix)
        {
            add(matrix);
            return *this;
        }

        inline const Matrix operator-(const Matrix& matrix) const
        {
            Matrix result(*this);
            subtract(result, matrix, result);
            return result;
        }

        inline Matrix& operator-=(const Matrix& matrix)
        {
            subtract(matrix);
            return *this;
        }

        inline const Matrix operator-() const
        {
            Matrix result(*this);
            negate(result);
            return result;
        }

        inline const Matrix operator*(const Matrix& matrix) const
        {
            Matrix result(*this);
            multiply(result, matrix, result);
            return result;
        }

        inline Matrix& operator*=(const Matrix& matrix)
        {
            multiply(matrix);
            return *this;
        }

        inline bool operator==(const Matrix& matrix)
        {
            for (size_t i = 0; i < C * R; ++i)
                if (m[i] != matrix.m[i])
                    return false;

            return true;
        }

        inline bool operator!=(const Matrix& matrix)
        {
            for (size_t i = 0; i < C * R; ++i)
                if (m[i] != matrix.m[i])
                    return true;

            return false;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        inline const Vector<3, T> operator*(const Vector<3, T>& v)
        {
            Vector<3, T> x;
            transformVector(v, x);
            return x;
        }

        template<size_t X = C, size_t Y = R, typename std::enable_if<(X == 4 && Y == 4)>::type* = nullptr>
        inline const Vector<4, T> operator*(const Vector<4, T>& v)
        {
            Vector<4, T> x;
            transformVector(v, x);
            return x;
        }
    };

    template<class T>
    inline Vector<3, T>& operator*=(Vector<3, T>& v, const Matrix<4, 4, T>& m)
    {
        m.transformVector(v);
        return v;
    }

    template<class T>
    inline Vector<4, T>& operator*=(Vector<4, T>& v, const Matrix<4, 4, T>& m)
    {
        m.transformVector(v);
        return v;
    }

    using Matrix4F = Matrix<4, 4, float>;
}

#endif // OUZEL_MATH_MATRIX_HPP
