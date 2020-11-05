#pragma once
class Color
{
public:
	Color() : r(0.f),g(0.f),b(0.f),a(0.f){}
	Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

	void Set(float r, float g, float b, float a = 1.0f)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	float* operator & ()
	{
		return (float*)this;
	}

	static Color Black() { return Color(0, 0, 0, 1); };
	static Color Blue() { return Color(0, 0, 1, 1); };
	static Color Clear() { return Color(0, 0, 0, 0); };
	static Color Cyan() { return Color(0, 1, 1, 1); };
	static Color Gray() { return Color(0.5F, 0.5F, 0.5F, 1); };
	static Color Green() { return Color(0, 1, 0, 1); };
	static Color Grey() { return Color(0.5F, 0.5F, 0.5F, 1); };
	static Color Magenta() { return Color(1, 0, 1, 1); };
	static Color Red() { return Color(1, 0, 0, 1); };
	static Color White() { return Color(1, 1, 1, 1); };
	static Color Yellow() { return Color(1, 0.92F, 0.016F, 1); };
	static Color Orange() { return Color(1, 0.5F, 0, 1); };
	static Color Purple() { return Color(0.5F, 0, 0.5F, 1); };

public:
	float r, g, b, a;
};