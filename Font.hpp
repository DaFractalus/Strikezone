#pragma once
#ifndef FONT_HPP
#define FONT_HPP

#include "SafeGL.hpp"

using namespace SGL;
namespace TLB
{
	struct Char
	{
		glm::ivec2   Size;       // Size of glyph
		glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
		uint Advance;    // Offset to advance to next glyph
		uint widthCumul; // X pixel Offset in bitmap
		uint row; // Y row offset in bitmap
	};

	struct TextVert
	{
		glm::vec2 pos;
		glm::vec2 tex;
		glm::vec4 col;
	};

	class Font
	{
		bool _loaded;
		int _capacity;
		int _charheight;
		int _side;

	public:
		SmartGLID _texture;
		std::unordered_map<uint, Char> chars;

		Font(int atlasPow = 10);
		bool Load(const char* path, uint32_t height);
		operator GLuint() const { return _texture; }
		void GlBindBitmapTex();
		void GlActivateBindBitmapTex(unsigned int location);
		vector<TextVert> RenderText(std::string text, glm::vec2 pos, float scale, glm::vec2 textOrigin, glm::vec4 tlCol, glm::vec4 trCol, glm::vec4 blCol, glm::vec4 brCol, glm::vec2* textRect = nullptr);
	};
}

#endif