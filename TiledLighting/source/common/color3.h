#pragma once



class color3
{
public:
	float r;
	float g;
	float b;

public:
	operator float* ();
	operator const float* () const;

	const color3& operator = (const color3&);
	const color3& operator = (unsigned int);

	color3& operator += (const color3&);
	color3& operator -= (const color3&);
	color3& operator *= (float);
	color3& operator /= (float);

	color3 operator + () const;
	color3 operator - () const;

	color3 operator + (const color3&) const;
	color3 operator - (const color3&) const;
	color3 operator * (float) const;
	color3 operator / (float) const;

	friend color3 operator * (float, const color3&);
	friend color3 operator / (float, const color3&);

	operator unsigned int () const;

	bool operator == (const color3&) const;
	bool operator != (const color3&) const;

public:
	color3(){}
	color3(const float* pf3) : r(pf3[0]), g(pf3[1]), b(pf3[2]) {}
	color3(const color3& other) : r(other.r), g(other.g), b(other.b) {}
	color3(float _r, float _g, float _b) : r(_r), g(_g), b(_b) {}
	color3(unsigned int uColor);
};
