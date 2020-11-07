#pragma once
#include "MemoryStream.h"
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
	static Color ServerColor() { return Color(0, 0, 1, 1); };
	static Color Clear() { return Color(0, 0, 0, 0); };
	static Color Cyan() { return Color(0, 1, 1, 1); };
	static Color Gray() { return Color(0.5F, 0.5F, 0.5F, 1); };
	static Color Green() { return Color(0, 1, 0, 1); };
	static Color WelcomeColor() { return Color(0, 1, 0, 1); };
	static Color Grey() { return Color(0.5F, 0.5F, 0.5F, 1); };
	static Color Magenta() { return Color(1, 0, 1, 1); };
	static Color Red() { return Color(1, 0, 0, 1); };
	static Color White() { return Color(1, 1, 1, 1); };
	static Color ChatColor() { return Color(1, 1, 1, 1); };
	static Color Yellow() { return Color(1, 0.92F, 0.016F, 1); };
	static Color Orange() { return Color(1, 0.5F, 0, 1); };
	static Color Purple() { return Color(0.5F, 0, 0.5F, 1); };
	static Color WhisperColor() { return Color(0.5F, 0, 0.5F, 1); };

	void Write(OutputMemoryStream& stream) const {
		stream.Write(r);
		stream.Write(g);
		stream.Write(b);
		stream.Write(a);
	}

	void Read(const InputMemoryStream& stream) {
		stream.Read(r);
		stream.Read(g);
		stream.Read(b);
		stream.Read(a);
	}

public:
	float r, g, b, a;
};