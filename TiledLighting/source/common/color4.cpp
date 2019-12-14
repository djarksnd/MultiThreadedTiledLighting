#include "color4.h"
#include "MathHelper.h"



color4::color4(unsigned int uColor)
:r(static_cast<float>((uColor >> 24) & 0xff) / 255.0f)
,g(static_cast<float>((uColor >> 16) & 0xff) / 255.0f)
,b(static_cast<float>((uColor >>  8) & 0xff) / 255.0f)
,a(static_cast<float>(uColor & 0xff) / 255.0f) 
{}




color4::operator float* ()
{
	return reinterpret_cast<float*>(this);
}



color4::operator const float* () const
{
	return reinterpret_cast<const float*>(this);
}



const color4& 
color4::operator = (const color4& other)
{
	r = other.r;
	g = other.g;
	b = other.b;
	a = other.a;

	return (*this);
}



const color4&
color4::operator = (unsigned int uColor)
{
	r = static_cast<float>((uColor >> 24) & 0xff) / 255.0f;
	g = static_cast<float>((uColor >> 16) & 0xff) / 255.0f;
	b = static_cast<float>((uColor >>  8) & 0xff) / 255.0f;
	a = static_cast<float>(  uColor         & 0xff) / 255.0f;

	return (*this);
}



color4& 
color4::operator += (const color4& other)
{
	r += other.r;
	g += other.g;
	b += other.b;
	a += other.a;
	return (*this);
}



color4& 
color4::operator -= (const color4& other)
{
	r -= other.r;
	g -= other.g;
	b -= other.b;
	a -= other.a;
	return (*this);
}



color4& 
color4::operator *= (const color4& other )
{
	r *= other.r;
	g *= other.g;
	b *= other.b;
	a *= other.a;
	return (*this);
}



color4& 
color4::operator /= (const color4& other )
{
	r /= other.r;
	g /= other.g;
	b /= other.b;
	a /= other.a;
	return (*this);
}



color4& 
color4::operator *= (float f)
{
	r *= f;
	g *= f;
	b *= f;
	a *= f;
	return (*this);
}



color4& 
color4::operator /= (float f)
{
	r /= f;
	g /= f;
	b /= f;
	a /= f;
	return (*this);
}

color4 
color4::operator + () const
{
	return (*this);
}



color4 
color4::operator - () const
{
	return color4(-r, -g, -b, -a);
}



color4 
color4::operator + (const color4& other) const
{
	return color4(*this) += other;
}



color4 
color4::operator - (const color4& other) const
{
	return color4(*this) -= other;
}



color4 
color4::operator * (const color4& other ) const
{
	return color4(*this) *= other;
}



color4 
color4::operator / (const color4& other ) const
{
	return color4(*this) /= other;
}



color4 
color4::operator * (float f) const
{
	return color4(*this) *= f;
}



color4
color4::operator / (float f) const
{
	return color4(*this) /= f;
}



color4 
operator * (float f, const color4& other)
{
	return other * f;
}



color4 
operator / (float f, const color4& other)
{
	return other / f;
}



color4::operator unsigned int () const
{
	unsigned int uColor  =   static_cast<unsigned int>(a * 255.0f + 0.5f) & 0xff;
				 uColor |= (static_cast<unsigned int>(b * 255.0f + 0.5f) & 0xff) << 8;
				 uColor |= (static_cast<unsigned int>(g * 255.0f + 0.5f) & 0xff) << 16;
				 uColor |= (static_cast<unsigned int>(r * 255.0f + 0.5f) & 0xff) << 24;

	return uColor;
}



bool 
color4::operator == (const color4& other) const
{
	return (other.r == r && other.g == g && other.b == b && other.a == a);
}



bool
color4::operator != (const color4& other) const
{
	return ! (other == (*this));
}
