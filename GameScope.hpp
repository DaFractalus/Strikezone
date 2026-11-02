#pragma once

#include "Renderkit.hpp"
#include "Assets.hpp"

using namespace TLB;
namespace Engine
{
	class GameEngine;

	class GameScope
	{
	public:
		std::string name;
		RenderBuffer renderBfr;
		GameEngine* engine;
		AssetManager* assets;
		bool _template;

		GameScope(GameEngine& engine);
		GameScope();
		virtual ~GameScope();
		virtual void ProcessInput();
		virtual void Update();
		virtual void Render();
		virtual void KeyCallNotice(const GLFWwindow* window, int key, int scancode, int action, int mods);
		virtual void MouseButtonCallNotice(const GLFWwindow* window, int button, int action, int mods);
		virtual void CursorPosCallNotice(const GLFWwindow* window, double xpos, double ypos);
		virtual void PackUI();
		virtual void NotifyBurried();
		virtual void NotifySurfaced();
		virtual void Terminate();
		virtual void ResetRenderTarget();


	};
}