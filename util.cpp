#include "util.hpp"

Vector2 operator+(const Vector2 &a, const Vector2 &b) { return (Vector2) { a.x + b.x, a.y + b.y }; }
Vector2 operator+=(Vector2 &a, const Vector2 &b) { a.x += b.x; a.y += b.y; return a; }
Vector2 operator-(const Vector2 &a, const Vector2 &b) { return (Vector2) { a.x - b.x, a.y - b.y }; }
Vector2 operator-(const Vector2 &a) { return (Vector2) { -a.x, -a.y }; }
Vector2 operator-=(Vector2 &a, const Vector2 &b) { a.x -= b.x; a.y -= b.y; return a; }
template<typename T> Vector2 operator*(const Vector2 &a, const T &b) { return (Vector2) { a.x * b, a.y * b }; }
template<typename T> Vector2 operator*=(Vector2 &a, const T &b) { a.x * b; a.y * b; return a; }
template<typename T> Vector2 operator/(const Vector2 &a, const T &b) { return (Vector2) { a.x / b, a.y / b }; }
template<typename T> Vector2 operator/=(Vector2 &a, const T &b) { a.x / b; a.y / b; return a; }

Color ColorLerp(Color dst, Color src, float range)
{
	return (Color){ (unsigned char)Lerp(dst.r, src.r, range), (unsigned char)Lerp(dst.g, src.g, range), (unsigned char)Lerp(dst.b, src.b, range), (unsigned char)Lerp(dst.a, src.a, range) };
}

void DrawTextCenter(Font font, const char *text, float x, float y, float size, float spacing, Color color)
{
	Vector2 tSize = { 0 };
	float offset = size / 10.0f;
	tSize = MeasureTextEx(font, text, size, spacing);
	DrawTextEx(font, text, (Vector2){ x - tSize.x / 2.0f, y - tSize.y / 2.0f + offset }, size, spacing, color);
}