#include "Vector3D.h"

Vector3D::Vector3D()
{
    x = 0;
    y = 0;
    z = 0;
}

Vector3D::Vector3D(float x1, float y1, float z1)
{
    x = x1;
    y = y1;
    z = z1;
}

Vector3D::Vector3D(const Vector3D& vec)
{
    x = vec.x;
    y = vec.y;
    z = vec.z;
}

Vector3D Vector3D::operator+(const Vector3D& vec)
{
    return Vector3D(x + vec.x, y + vec.y, z + vec.z);
}

Vector3D& Vector3D::operator+=(const Vector3D& vec)
{
    x += vec.x;
    y += vec.y;
    z += vec.z;
    return *this;
}

Vector3D Vector3D::operator-(const Vector3D& vec)
{
    return Vector3D(x - vec.x, y - vec.y, z - vec.z);
}

Vector3D& Vector3D::operator-=(const Vector3D& vec)
{
    x -= vec.x;
    y -= vec.y;
    z -= vec.z;
    return *this;
}

Vector3D Vector3D::operator*(float value)
{
    return Vector3D(x * value, y * value, z * value);
}

Vector3D& Vector3D::operator*=(float value)
{
    assert(value != 0);
    x *= value;
    y *= value;
    z *= value;
    return *this;
}
Vector3D Vector3D::operator/(float value)
{
    return Vector3D(x / value, y / value, z / value);
}

Vector3D& Vector3D::operator/=(float value)
{
    assert(value != 0);
    x /= value;
    y /= value;
    z /= value;
    return *this;
}

Vector3D& Vector3D::operator=(const Vector3D& vec)
{
    x = vec.x;
    y = vec.y;
    z = vec.z;
    return *this;
}

float Vector3D::dot_product(const Vector3D& vec) const
{
    return (x * vec.x + y * vec.y + z * vec.z);
}

Vector3D Vector3D::cross_product(const Vector3D& vec) const
{
    float ni = y * vec.z - z * vec.y;
    float nj = z * vec.x - x * vec.z;
    float nk = x * vec.y - y * vec.x;
    return Vector3D(ni, nj, nk);
}

Vector3D Vector3D::normalised()
{
    return Vector3D(*this/magnitude());
}

float Vector3D::square() const
{
    return x * x + y * y + z * z;
}

float Vector3D::distance(const Vector3D& vec) const
{
    return sqrt(square() - vec.square());
}

float Vector3D::magnitude() const
{
    return sqrt(square());
}

float Vector3D::get_x() const
{
    return x;
}

float Vector3D::get_y() const
{
    return y;
}

float Vector3D::get_z() const
{
    return z;
}

void Vector3D::disp() const
{
    cout << x << " " << y << " " << z << endl;
}
