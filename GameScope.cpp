#include "GameScope.hpp"
#include "GameEngine.hpp"

namespace Engine
{
	GameScope::GameScope(GameEngine& engine) : name("_template_engine_"), engine(&engine), assets(engine.assets.get())
	{
		_template = false;
	}

	GameScope::GameScope() : name(""), engine(nullptr), assets(nullptr)
	{
		_template = true;
	}

	void GameScope::ProcessInput()
	{

	}

	void GameScope::KeyCallNotice(const GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		(void)window; (void)key; (void)scancode; (void)action; (void)mods;
	}

	void GameScope::MouseButtonCallNotice(const GLFWwindow* window, int button, int action, int mods)
	{

	}

	void GameScope::CursorPosCallNotice(const GLFWwindow* window, double xpos, double ypos)
	{

	}

	void GameScope::Update()
	{

	}

	void GameScope::Render()
	{

	}

	void GameScope::PackUI()
	{

	}

	void GameScope::ResetRenderTarget()
	{
		if (engine)
		{
			glm::ivec2 dims = engine->Video().displayDims;
			renderBfr.Resize(dims.x, dims.y);
		}
	}

	void GameScope::NotifyBurried()
	{

	}

	void GameScope::NotifySurfaced()
	{

	}

	void GameScope::Terminate()
	{

	}

	GameScope::~GameScope()
	{
		if (name.size() > 0) std::cout << "[" << name << "] Scope Closed\n";
	}
}