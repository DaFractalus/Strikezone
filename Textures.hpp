#pragma once
#ifndef TEXTURES_HPP
#define TEXTURES_HPP

#include "SafeGL.hpp"

using namespace SGL;
namespace TLB
{
	//- Texture -
	class Texture
	{
	public:
		SmartGLID ID;
		int width, height, nrChannels;
		bool loaded;
		std::string type;
		std::string path;


		Texture();
		void Load(const std::string& png_path, bool inSRGB = true, bool flipped = false);
		bool GlBindTexture() const;
		bool GlActivateBindTexture(unsigned int location) const;
		int getWidth() const;
		int getHeight() const;

	};

	//- CubeMap -
	class CubeMap
	{
		SmartGLID ID;
	public:
		bool loaded;

		CubeMap();
		void Load(vector<std::string> faces, bool inSRGB = true);
		bool GlBindCubeMap();
		bool GlActivateBindCubeMap(unsigned int location);

	};

}


#endif