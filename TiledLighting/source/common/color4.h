#pragma once



#include "color3.h"



class color4
{
public:
	float r;
	float g;
	float b;
	float a;

public:
	operator float* ();
	operator const float* () const;

	const color4& operator = (const color4&);
	const color4& operator = (unsigned int);

	color4& operator += (const color4&);
	color4& operator -= (const color4&);
	color4& operator *= (const color4&);
	color4& operator /= (const color4&);
	color4& operator *= (float);
	color4& operator /= (float);

	color4 operator + () const;
	color4 operator - () const;

	color4 operator + (const color4&) const;
	color4 operator - (const color4&) const;
	color4 operator * (const color4&) const;
	color4 operator / (const color4&) const;
	color4 operator * (float) const;
	color4 operator / (float) const;

	friend color4 operator * (float, const color4&);

	operator unsigned int () const;

	bool operator == (const color4&) const;
	bool operator != (const color4&) const;

public:
	color4(){}
	color4(const float* pf4): r(pf4[0]), g(pf4[1]), b(pf4[2]), a(pf4[3]){}
	color4(const color4& _other) : r(_other.r), g(_other.g), b(_other.b), a(_other.a) {}
	color4(const color3& _color, float _fAlpha) : r(_color.r), g(_color.g), b(_color.b), a(_fAlpha) {}
	color4(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
	color4(unsigned int color);
};

