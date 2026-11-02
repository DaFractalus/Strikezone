#include "StartMenu.hpp"
#include "GameEngine.hpp"

namespace Engine
{
	StartMenu::StartMenu(GameEngine& engine) : GameScope(engine),
		frameCounter(0), timeCounter(0), menuTrack(assets->audio), uicanvas(engine)
	{
		name = "Start Menu";
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glm::ivec2 scrDims = engine.Video().displayDims;
		vector<RBFormatParam> formatParams = { { GL_RGBA8, GL_RGBA,  GL_FLOAT} };
		renderBfr.Create(scrDims.x, scrDims.y, true, formatParams);

		//
		//menuTrack.SetTrack(3);
		//menuTrack.SetPitch(.9f);
	}

	void StartMenu::KeyCallNotice(const GLFWwindow* window, int key, int scancode, int action, int mods)
	{

	}

	void StartMenu::MouseButtonCallNotice(const GLFWwindow* window, int button, int action, int mods)
	{

	}

	void StartMenu::ProcessInput()
	{
		uicanvas.DispatchInput();
	}

	void StartMenu::Update()
	{
		//Updates
		timeCounter += engine->deltaTime > .5f ? 0 : (float)engine->deltaTime;
		frameCounter++;
	}

	void StartMenu::Render()
	{
		//Binding Render Target
		if (!renderBfr.StartRender()) return;

		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		//glClearColor(0.2f, 0.4f, 0.1f, 1);
		glClearColor(0.22f, 0.24f, 0.24f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		uicanvas.RenderContent(engine->Video());

	}

	void StartMenu::PackUI()
	{
		uicanvas.StartFrame();

		auto pair = uicanvas.CallWindow(0);
		auto win = pair.first;
		if (pair.second)
		{
			win->title = "Test Window (This is kinda it for now, much to add later)";
			win->size = { 800, 500 };
		}

		{
			auto btnpair = win->CallButton(0);
			auto btn = btnpair.first;
			if (btnpair.second)
			{
				btn->title = "Short Button";
				btn->length = 4 * FC;
			}
			if (btn->clicked) std::cout << "short button tickled\n";
		}

		{
			auto btnpair = win->CallButton(1);
			auto btn = btnpair.first;
			if (btnpair.second)
			{
				btn->title = "Long Button";
				btn->length = 6 * FC;
			}
			if (btn->clicked) std::cout << "long button tapped\n";
		}

		{
			auto btnpair = win->CallButton(2);
			auto btn = btnpair.first;
			if (btnpair.second)
			{
				btn->title = "Even Longer Button";
				btn->length = 8 * FC;
			}
			if (btn->clicked) std::cout << "even longer button punched\n";
		}

		{
			auto btnpair = win->CallButton(3);
			auto btn = btnpair.first;
			if (btnpair.second)
			{
				OSS oss; oss << " FPS: " << 1 / engine->deltaTime;
				btn->title = oss.str();
				btn->length = 8*FC;
			}
			if (btn->clicked) std::cout << "*nothing happens*\n";
		}
		

		uicanvas.EndFrame();

	}

	void StartMenu::NotifyBurried()
	{
		menuTrack.SetTrack(-1);
	}

	void StartMenu::NotifySurfaced()
	{
		//menuTrack.SetTrack(3);
	}

	void StartMenu::Terminate()
	{

	}
}