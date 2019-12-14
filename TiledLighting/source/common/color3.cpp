#include "color3.h"
#include "MathHelper.h"


color3::color3(unsigned int uColor)
:r(static_cast<float>((uColor >> 24) & 0xff) / 255.0f)
,g(static_cast<float>((uColor >> 16) & 0xff) / 255.0f)
,b(static_cast<float>((uColor >> 8) & 0xff) / 255.0f)
{}



color3::operator float* ()
{
	return reinterpret_cast<float*>(this);
}



color3::operator const float* () const
{
	return reinterpret_cast<const float*>(this);
} 



const color3& 
color3::operator = (const color3& other)
{
	r = other.r;
	g = other.g;
	b = other.b;

	return (*this);
}



const color3&
color3::operator = (unsigned int uColor)
{
	r = static_cast<float>((uColor >> 24) & 0xff) / 255.0f;
	g = static_cast<float>((uColor >> 16) & 0xff) / 255.0f;
	b = static_cast<float>((uColor >>  8) & 0xff) / 255.0f;

	return (*this);
}



color3& 
color3::operator += (const color3& other)
{
	r += other.r;
	g += other.g;
	b += other.b;
	return (*this);
}



color3& 
color3::operator -= (const color3& other)
{
	r -= other.r;
	g -= other.g;
	b -= other.b;
	return (*this);
}



color3& 
color3::operator *= (float f)
{
	r *= f;
	g *= f;
	b *= f;
	return (*this);
}



color3& 
color3::operator /= (float f)
{
	r /= f;
	g /= f;
	b /= f;
	return (*this);
}



color3 
color3::operator + () const
{
	return (*this);
}



color3 
color3::operator - () const
{
	return color3(-r, -g, -b);
}



color3
color3::operator + (const color3& other) const
{
	return color3(*this) += other;
}



color3
color3::operator - (const color3& other) const
{
	return color3(*this) -= other;
}



color3
color3::operator * (float f) const
{
	return color3(*this) *= f;
}



color3 
color3::operator / (float f) const
{
	return color3(*this) /= f;
}



color3 
operator * (float f, const color3& other)
{
	return other * f;
}



color3 
operator / (float f, const color3& other)
{
	return other / f;
}



color3::operator unsigned int () const
{
	unsigned int uColor  = (static_cast<unsigned int>(b * 255.0f + 0.5f) & 0xff) << 8;
				 uColor |= (static_cast<unsigned int>(g * 255.0f + 0.5f) & 0xff) << 16;
				 uColor |= (static_cast<unsigned int>(r * 255.0f + 0.5f) & 0xff) << 24;

	return uColor;
}



bool 
color3::operator == (const color3& other) const
{
	return (other.r == r && other.g == g && other.b == b);
}



bool
color3::operator != (const color3& other) const
{
	return ! (other == (*this));
}
