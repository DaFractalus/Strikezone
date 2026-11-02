#pragma once
#ifndef ATLAS_HPP
#define ATLAS_HPP

#include "SafeGL.hpp"

using namespace SGL;
namespace TLB
{
	//
	struct SpriteRect
	{
		int X, Y, W, H, EXT_W, EXT_H;

		SpriteRect(int x, int y, int w, int h, int ext_w, int ext_h);
		int getOriginW(int unit) const, getOriginH(int unit) const;

	};

	//Atlas
	class Atlas
	{
		int _unit;
		int _capacity;
		int _levels;
		SmartGLID _texture;
		std::unordered_map<int, std::unique_ptr<SpriteRect>> _sprites;
		std::vector<bool> _cellState;

	public:

		Atlas(int unit, int width_capacity, bool is_srgb);
		const SpriteRect& GetSprite(int id) const;
		bool PackTexture(int id, std::string path);
		bool PackImageData(int id, int width, int height, int channels, uint8_t* data);
		void GlBindAtlasTex();
		void GlActivateBindAtlasTex(unsigned int location);
		int GetUnit() const;
		int GetCapacity() const;
		operator GLuint() const { return _texture; }
	};
}

#endif