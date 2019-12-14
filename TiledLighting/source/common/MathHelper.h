#pragma once

namespace MathHelper
{
	constexpr float FloatEpsilon = 1.0e-6f;
	constexpr float PI = 3.1415926535897932384626433832795f;
	constexpr float HalfPI = 1.5707963267948966192313216916398f;
	constexpr float TwoPI = 6.283185307179586476925286766559f;
	constexpr float Float_Max = 3.402823466e+38F;
	constexpr float Float_Min = 1.175494351e-38F;

	constexpr float
		DegreeToRadian(float _degree)
	{
		return _degree * (PI / 180.f);
	}

	constexpr float
		RadianToDegree(float _radian)
	{
		return _radian * (180.f / PI);
	}

	constexpr bool
		FloatIsNearlyZero(float a)
	{
		return (a < FloatEpsilon && a > -FloatEpsilon);
	}

	constexpr bool
		FloatsAreEqual(float a, float b)
	{
		return (FloatIsNearlyZero(a - b));
	}

	template<typename T>
	constexpr const T
		Max(const T& lhs, const T& rhs)
	{
		return lhs > rhs ? lhs : rhs;
	}

	template<typename T>
	constexpr const T
		Min(const T& lhs, const T& rhs)
	{
		return lhs < rhs ? lhs : rhs;
	}

	template<typename T>
	constexpr const T
		Clamp(const T& var, const T& min, const T& max)
	{
		return Min(Max(var, min), max);
	}

	template<typename T>
	constexpr const T
		Lerp(const T& start, const T& end, float t)
	{
		if (start == end || FloatsAreEqual(t, 0.0f))
		{
			return start;
		}

		if (FloatsAreEqual(t, 1.0f))
		{
			return end;
		}

		return ((1.0f - t) * start) + (t * end);
	}
};