#pragma once

#include <dg/core/common.hpp>

namespace dg {

class Color
{
public:
	//float r,g,b,a;
	float r,g,b,a;

	Color() = default;
	constexpr Color(float r, float g, float b, float a=1.0) : r(r), g(g), b(b), a(a) {}

	Color operator-(const Color& right) const
	{
		return Color(r - right.r, g - right.g, b - right.b, a - right.a);
	}

	Color operator-() const
	{
		return Color(-r, -g, -b, -a);
	}

	Color& operator-=(const Color& right)
	{
		r -= right.r;
		g -= right.g;
		b -= right.b;
		a -= right.a;
		return *this;
	}

	Color operator+(const Color& right) const
	{
		return Color(r + right.r, g + right.g, b + right.b, a + right.a);
	}

	Color& operator+=(const Color& right)
	{
		r += right.r;
		g += right.g;
		b += right.b;
		a += right.a;
		return *this;
	}

	Color operator*(float s) const
	{
		return Color(r * s, g * s, b * s, a * s);
	}

	Color& operator*=(float s)
	{
		r *= s;
		g *= s;
		b *= s;
		a *= s;
		return *this;
	}

	Color operator*(const Color& right) const
	{
		return Color(r * right.r, g * right.g, b * right.b, a * right.a);
	}

	Color& operator*=(const Color& right)
	{
		r *= right.r;
		g *= right.g;
		b *= right.b;
		a *= right.a;
		return *this;
	}

	Color operator/(float s) const
	{
		return Color(r / s, g / s, b / s, a / s);
	}

	Color& operator/=(float s)
	{
		r /= s;
		g /= s;
		b /= s;
		a /= s;
		return *this;
	}

	Color operator/(const Color& right) const
	{
		return Color(r / right.r, g / right.g, b / right.b, a / right.a);
	}

	Color& operator/=(const Color& right)
	{
		r /= right.r;
		g /= right.g;
		b /= right.b;
		a /= right.a;
		return *this;
	}

	bool operator==(const Color& right) const
	{
		return r == right.r && g == right.g && b == right.b && a == right.a;
	}

	bool operator!=(const Color& right) const
	{
		return !(*this == right);
	}

	Color& operator=(const Color&) = default;

	Color operator<(const Color& right) const
	{
		return Color(r < right.r ? 1.0f : 0.0f,
					 g < right.g ? 1.0f : 0.0f,
					 b < right.b ? 1.0f : 0.0f,
					 a < right.a ? 1.0f : 0.0f);
	}

	Color operator>(const Color& right) const
	{
		return Color(r > right.r ? 1.0f : 0.0f,
					 g > right.g ? 1.0f : 0.0f,
					 b > right.b ? 1.0f : 0.0f,
					 a > right.a ? 1.0f : 0.0f);
	}

	Color operator<=(const Color& right) const
	{
		return Color(r <= right.r ? 1.0f : 0.0f,
					 g <= right.g ? 1.0f : 0.0f,
					 b <= right.b ? 1.0f : 0.0f,
					 a <= right.a ? 1.0f : 0.0f);
	}

	Color operator>=(const Color& right) const
	{
		return Color(r >= right.r ? 1.0f : 0.0f,
					 g >= right.g ? 1.0f : 0.0f,
					 b >= right.b ? 1.0f : 0.0f,
					 a >= right.a ? 1.0f : 0.0f);
	}

	float* data() { return reinterpret_cast<float*>(this); }

	const float* data() const { return reinterpret_cast<const float*>(this); }

	float& operator[](size_t index)
	{
		return data()[index];
	}

	const float& operator[](size_t index) const
	{
		return data()[index];
	}

	friend std::ostream& operator<<(std::ostream& os, const Color& c)
	{
		os << c.r << " " << c.g << " " << c.b << " " << c.a;
		return os;
	}

	friend std::istream& operator>>(std::istream& is, Color& c)
	{
		is >> c.r >> c.g >> c.b >> c.a;
		return is;
	}

public:

	static float toSRGB(float v)
	{
		return std::pow(v, 2.2f);
	}

	static float fromSRGB(float v)
	{
		return std::pow(v, 1.0f/2.2f);
	}

	Color toSRGB() const
	{
		return Color(toSRGB(r), toSRGB(g), toSRGB(b), a);
	}

	Color fromSRGB() const
	{
		return Color(fromSRGB(r), fromSRGB(g), fromSRGB(b), a);
	}

	uint32_t toUInt32() const
	{
		uint8_t cr = r*255.0f;
		uint8_t cg = g*255.0f;
		uint8_t cb = b*255.0f;
		uint8_t ca = a*255.0f;
		return (cr << 0) + (cg << 8) + (cb << 16) + (ca << 24);
	}

	static dg::Color fromHexString(std::string s)
	{
		dg::Color color(0.0f,0.0f,0.0f,1.0f);

		if(s.empty())
			return color;

		if(s[0]=='#') // remove an optional # at the beginning
			s = s.substr(1);

		std::string sub;
		switch(s.length())
		{
			case 3: // #rgb
				// #rgb is interpret as #rrggbb (hence double each character)
				sub = s.substr(0,1);
				color.r = hex_to_int(sub+sub) / 255.0f;
				sub = s.substr(1,1);
				color.g = hex_to_int(sub+sub) / 255.0f;
				sub = s.substr(2,1);
				color.b= hex_to_int(sub+sub) / 255.0f;
				break;
			case 4: // #rgba
				// #rgba is interpret as #rrggbbaa (hence double each character)
				sub = s.substr(0,1);
				color.r = hex_to_int(sub+sub) / 255.0f;
				sub = s.substr(1,1);
				color.g = hex_to_int(sub+sub) / 255.0f;
				sub = s.substr(2,1);
				color.b= hex_to_int(sub+sub) / 255.0f;
				sub = s.substr(3,1);
				color.a= hex_to_int(sub+sub) / 255.0f;
				break;
			case 6: // #rrggbb
				color.r = hex_to_int(s.substr(0,2)) / 255.0f;
				color.g = hex_to_int(s.substr(2,2)) / 255.0f;
				color.b = hex_to_int(s.substr(4,2)) / 255.0f;
				break;            
			case 8: // #rrggbb
				color.r = hex_to_int(s.substr(0,2)) / 255.0f;
				color.g = hex_to_int(s.substr(2,2)) / 255.0f;
				color.b = hex_to_int(s.substr(4,2)) / 255.0f;
				color.a = hex_to_int(s.substr(6,2)) / 255.0f;
				break;      
		}

		return color;
	}   

private:

	static int hex_to_int(const std::string& s)
	{
		std::stringstream ss;
		int val = 0;
		ss << std::hex << s;
		ss >> val;
		return val;
	};
};

namespace colors
{
	constexpr static Color White   = {1.0f, 1.0f, 1.0f, 1.0f};
	constexpr static Color Black   = {0.0f, 0.0f, 0.0f, 1.0f};
	constexpr static Color Red     = {1.0f, 0.0f, 0.0f, 1.0f};
	constexpr static Color Green   = {0.0f, 1.0f, 0.0f, 1.0f};
	constexpr static Color Blue    = {0.0f, 0.0f, 1.0f, 1.0f};
	constexpr static Color Yellow  = {1.0f, 1.0f, 0.0f, 1.0f};
	constexpr static Color Magenta = {1.0f, 0.0f, 1.0f, 1.0f};
	constexpr static Color Cyan    = {0.0f, 1.0f, 1.0f, 1.0f};
}

}
