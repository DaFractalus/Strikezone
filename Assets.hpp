#pragma once
#ifndef ASSETS_HPP
#define ASSETS_HPP

#include "Toolkit.hpp"
#include "Renderkit.hpp"
//#include "ModelComponents.hpp"
#include "Atlas.hpp" //TODO: add the better approach of texture arrays
#include "Font.hpp"
#include "Audio.hpp"
#include "Text.hpp"
//#include "AniModel.hpp"

namespace TLB
{
	struct MemMesh
	{
		std::vector<Vertex> vertices;
		std::vector<uint> indices;
		MemMesh(std::vector<Vertex> vertices, std::vector<uint> indices)
			: vertices(std::move(vertices)), indices(std::move(indices)) {
		}
	};

	class AssetManager
	{
	public:
		std::shared_mutex assetMutex;
		//---------------------
		Atlas uiatlas;
		Font uifont;
		//---------------------
		//CubeMap skybox;
		Shader engineScreenVF;
		Shader uiShader;
		Shader panelShader;
		Shader textShader;
		Shader uisepShader;
		//Atlas atlas;
		//TODO: use textureArrays; make a class which takes the role of the atlas,
		// this class manages multiple texture arrays, might even take on bindless textures too
		//--------------------
		//Atlas itematlas;
		AudioManager audio;
		Language lang;
		SmartGLID cursorVAO, cursorVBO;
		SmartGLID skyVAO, skyVBO;
		std::unordered_map<uint, MemMesh> tileMeshes;

		AssetManager();
		void GenerateTileMeshes();
		void LoadAssets();
	};
}

#endif