#pragma once

#include "GameScope.hpp"
#include "UIkit.hpp"

using namespace UIB;
namespace Engine
{
	class StartMenu : public GameScope
	{
	public:
		UICanvas uicanvas;
		std::size_t frameCounter;
		float timeCounter;
		TrackInstance menuTrack;

		StartMenu(GameEngine& engine);
		void ProcessInput() override;
		void KeyCallNotice(const GLFWwindow* window, int key, int scancode, int action, int mods) override;
		void MouseButtonCallNotice(const GLFWwindow* window, int button, int action, int mods) override;
		void Update() override;
		void Render() override;
		void PackUI() override;
		void NotifyBurried() override;
		void NotifySurfaced() override;
		void Terminate() override;
	};
}