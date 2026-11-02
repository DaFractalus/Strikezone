#pragma once
 
#include "SafeGL.hpp"
#include "Assets.hpp"
#include "Controls.hpp"
#include "GameScope.hpp"

using namespace SGL;
using namespace TLB;
namespace Engine
{
	struct VideoMode
	{
		uint targetFPS;
		bool vSync, fullscreen;
		glm::ivec2 screenDims; //stores the game's window dimension either fullscreen or windowed
		glm::ivec2 displayDims; //stores drawn game dimensions
		glm::ivec2 windowedDims, windowedPos; //stores windowed mode's dimensions
		bool fillMonitorDims;
		glm::ivec2 screenRatio;
		bool IGNORE_WIN_SIZE_CHANGES = false;
		bool windowFocused = false, windowHovered = false;
	};

	class GameEngine
	{
	private:
		VideoMode video;
		GameInput GMInput;

	public:
		GLFWwindow* window;
		SafeGLContext* sglcontext;
		std::unique_ptr<AssetManager> assets;
		std::stack<std::unique_ptr<GameScope>> scopeStack;
		size_t frameCount;
		double deltaTime;
		bool limitProcessGlTasks;

		GLuint screenVBO, screenVAO; 

		GameEngine(bool onMainThread);
		~GameEngine();

		int Run();
		void Shutdown();

		inline void RenderQuad() const
		{
			glBindVertexArray(screenVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}
		inline const VideoMode& Video() { return video; }
		inline GameInput& Input() { return GMInput; }

		// Callback Notifications
		void MouseButtonCallNotif(int button, int action, int mods);
		void ScrollCallNotif(double xoffset, double yoffset);
		void CursorPosCallNotif(double xpos, double ypos);
		void KeyCallNotif(int key, int scancode, int action, int mods);
		void UserEditedWindowSize(int width, int height);
		void UserFocusedWindow(bool state);
		void UserEditedWindowPos(int xpos, int ypos);

	private:
		bool Initialize(int scr_width, int scr_height);
		void LoadContent();
		void PushScope(std::unique_ptr<GameScope> scope);
		void PopScope();

		void ProcessInput();
		void Update();
		void Render();

		void SetDisplayDims(glm::ivec2 dims);
		void ToggleFullscreen();
		void RefreshDisplayDims();

		double HandleClock(double& lastTime) const;

	};

}