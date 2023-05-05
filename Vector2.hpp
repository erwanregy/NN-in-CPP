#pragma once

#include <iostream>
#include <type_traits>

template <typename type, std::enable_if<std::is_arithmetic<type>::value>::type* = nullptr>
struct Vector2 {
	type x, y;

	Vector2() : x(0), y(0) {}

	Vector2(type x, type y) : x(x), y(y) {}

	double magnitude() { return sqrt(x * x + y * y); }

	Vector2<double> normalise() { return Vector2<double>(x, y) / magnitude(); }

	Vector2 perpandicular() { return Vector2(-y, x); }

	Vector2 operator+(const Vector2& rhs) const { return Vector2(x + rhs.x, y + rhs.y); }

	Vector2& operator+=(const Vector2& rhs) { return *this = *this + rhs; }

	Vector2 operator-(const Vector2& rhs) const { return Vector2(x - rhs.x, y - rhs.y); }

	Vector2& operator-=(const Vector2& rhs) { return *this = *this - rhs; }

	Vector2 operator*(const Vector2& rhs) const { return Vector2(x * rhs.x, y * rhs.y); }

	Vector2 operator*(type rhs) const { return Vector2(x * rhs, y * rhs); }

	Vector2& operator*=(const Vector2& rhs) { return *this = *this * rhs; }

	Vector2& operator*=(type rhs) { return *this = *this * rhs; }

	Vector2 operator/(const Vector2& rhs) const { return Vector2(x / rhs.x, y / rhs.y); }

	Vector2 operator/(type rhs) const { return Vector2(x / rhs, y / rhs); }

	Vector2& operator/=(const Vector2& rhs) { return *this = *this / rhs; }

	Vector2& operator/=(type rhs) { return *this = *this / rhs; }

	bool operator<(const Vector2& rhs) const { return x < rhs.x and y < rhs.y; }

	bool operator<=(const Vector2& rhs) const { return x <= rhs.x and y <= rhs.y; }

	bool operator==(const Vector2& rhs) const { return x == rhs.x and y == rhs.y; }

	bool operator>=(const Vector2& rhs) const { return x >= rhs.x and y >= rhs.y; }

	bool operator>(const Vector2& rhs) const { return x > rhs.x and y > rhs.y; }

	operator std::string() { return '(' + x + ", " + y + ')'; }

	friend std::ostream& operator<<(std::ostream& stream, const Vector2<type>& vector2) { return stream << '(' << vector2.x << ", " << vector2.y << ')'; }

	friend std::istream& operator>>(std::istream& stream, Vector2<type>& vector2) { return stream >> vector2.x >> vector2.y; }

	friend std::wostream& operator<<(std::wostream& stream, const Vector2<type>& vector2) { return stream << vector2.x << L' ' << vector2.y;  }

	friend std::wistream& operator>>(std::wistream& stream, Vector2<type>& vector2) { return stream >> vector2.x >> vector2.y; }
};