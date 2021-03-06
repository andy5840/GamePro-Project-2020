#pragma once
//2차원 벡터를 나타내는 구조체입니다.
//여러 기본 연산을 지원합니다.

struct Vector2 {
	float x, y;

	Vector2();
	Vector2(float x, float y);

	Vector2 operator +(const Vector2& v) const;
	Vector2 operator -(const Vector2& v) const;
	Vector2 operator /(const Vector2& v) const;
	Vector2 operator *(const Vector2& v) const;

	Vector2 operator *(float f) const;
	Vector2 operator /(float f) const;

	Vector2 operator -(void) const;

	Vector2& operator +=(const Vector2& v);
	Vector2& operator -=(const Vector2& v);
	Vector2& operator /=(const Vector2& v);
	Vector2& operator *=(const Vector2& v);

	Vector2& operator +=(float f);
	Vector2& operator -=(float f);
	Vector2& operator *=(float f);
	Vector2& operator /=(float f);

	bool operator ==(const Vector2& v) const;
	bool operator !=(const Vector2& v) const;

	float DotProduct(const Vector2& v) const;

	void Translate(const Vector2& v);
	void Translate(float x, float y);

	void SetVector(const Vector2& v);
	void SetVector(float x, float y);

	float magnitude();
	Vector2 normalized();
};


#include "Vector2.inl"