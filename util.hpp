#pragma once

#include <raylib.h>
#include <raymath.h>
#include <cstring>

Vector2 operator+(const Vector2 &a, const Vector2 &b);
Vector2 operator+=(Vector2 &a, const Vector2 &b);
Vector2 operator-(const Vector2 &a, const Vector2 &b);
Vector2 operator-(const Vector2 &a);
Vector2 operator-=(Vector2 &a, const Vector2 &b);
template<typename T> Vector2 operator*(const Vector2 &a, const T &b);
template<typename T> Vector2 operator*=(Vector2 &a, const T &b);
template<typename T> Vector2 operator/(const Vector2 &a, const T &b);
template<typename T> Vector2 operator/=(Vector2 &a, const T &b);

Color ColorLerp(Color dst, Color src, float range);

void DrawTextCenter(Font font, const char *text, float x, float y, float size, float spacing, Color color);

inline void ArrayInit(void *array, int size)
{
	memset(array, size, 0);
}

/*
inline Vector2 Vector2Normalize(Vector2 a)
{
	float len = sqrt(a.x * a.x + a.y * a.y);
	a.x /= len;
	a.y /= len;
	return a;
}

inline float Vector2DotProduct(Vector2 a, Vector2 b)
{
	return a.x * b.x + a.y * b.y;
}
*/
