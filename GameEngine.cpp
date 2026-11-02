#include "GameEngine.hpp"
#include "GLFWCallbacks.hpp"
#include "StartMenu.hpp"

namespace Engine
{
	GameEngine::GameEngine(bool onMainThread) : window(nullptr), frameCount(0), deltaTime(0.0), limitProcessGlTasks(false),
		sglcontext(&IntializeSGLContext(onMainThread)), screenVAO(0), screenVBO(0),
		GMInput(*this)
	{}

	GameEngine::~GameEngine() { Shutdown(); }

	//
	int GameEngine::Run()
	{
		auto lastTime = glfwGetTime();
		if (!Initialize(1387, 795)) { return -1; }

		while (!glfwWindowShouldClose(window))
		{
			ProcessInput();
			Update();
			if (!glfwWindowShouldClose(window)) Render();
			ProcessQueueGLTasks(lastTime, video.targetFPS, limitProcessGlTasks);

			frameCount++;
			deltaTime = HandleClock(lastTime);
		}

		return 0;
	}

	void GameEngine::Shutdown()
	{
		if (window)
		{
			Safe_GlDeleteVertexArrays(1, &screenVAO);
			Safe_GlDeleteBuffers(1, &screenVBO);
			glfwDestroyWindow(window);
			while (!scopeStack.empty()) scopeStack.pop();
			assets = nullptr;
			window = nullptr;
		}
		glfwTerminate();

		std::cout << "[GameEngine] Successful Shutdown.\n";

		std::cout << "\n[Exit Log]:\n";
		SGL::PrintoutActiveCounts();
		SGL::DestroySGLContext();
	}

	// Callback Notifications
	void GameEngine::KeyCallNotif(int key, int scancode, int action, int mods)
	{
		//auto topScope = scopeStack.top().get();
		if (GMInput.WasKeyClicked(KeyBind::ReturnBack, key, action))
		{
			//if (showAdvWin) { showAdvWin = false; }
			//else if (showSimList && !staticSimList) { showSimList = false; }
			//else
			//{
			//	PopEngine();
			//	topScope = scopeStack.top().get();
			//}
		}

		else if (GMInput.WasKeyClicked(KeyBind::Pause, key, action))
		{
			//running = !running && !topScope->_template;
		}

		else if (WasKeyPressed(GLFW_KEY_F1, key, action))
		{
			//showSimList = !showSimList;
		}

		else if (GMInput.WasKeyClicked(KeyBind::ToggleDebug, key, action))
		{
			//showDebugWindow = !showDebugWindow; //TODO: toggle some other type of debug window
		}

		else if (GMInput.WasKeyClicked(KeyBind::ToggleFullscreen, key, action))
		{
			ToggleFullscreen();
		}

		//if (!topScope->_template)
		//{
		//	topScope->KeyCallNotice(window, key, scancode, action, mods);
		//}

	}

	void GameEngine::MouseButtonCallNotif(int button, int action, int mods)
	{
		//if (!scopeStack.top()->_template)
		//{
		//	scopeStack.top()->MouseButtonCallNotice(window, button, action, mods);
		//}
	}

	void GameEngine::ScrollCallNotif(double xoffset, double yoffset)
	{
		GMInput.mouseScroll = yoffset; (void)xoffset;
	}

	void GameEngine::CursorPosCallNotif(double xpos, double ypos)
	{
		GMInput.UpdateCursorPos(xpos, ypos);
		//if (!scopeStack.top()->_template)
		//{
		//	scopeStack.top()->CursorPosCallNotice(window, xpos, ypos);
		//}
	}

	void GameEngine::UserEditedWindowSize(int width, int height)
	{
		video.screenDims.x = width, video.screenDims.y = height;
		if (!video.fullscreen)
		{
			video.windowedDims.x = width, video.windowedDims.y = height;
			RefreshDisplayDims();
		}
	}

	void GameEngine::UserEditedWindowPos(int xpos, int ypos)
	{
		if (!video.fullscreen)
		{
			video.windowedPos.x = xpos, video.windowedPos.y = ypos;
		}
	}

	void GameEngine::UserFocusedWindow(bool state)
	{
		video.windowFocused = state;
	}

	//
	bool GameEngine::Initialize(int scr_width, int scr_height)
	{
		// Temporary VideoMode values:
		video.screenDims = { scr_width, scr_height };
		video.windowedDims = video.screenDims;
		video.targetFPS = 60;
		video.fillMonitorDims = true;
		video.fullscreen = false;
		video.screenRatio = glm::ivec2(16, 9);
		video.vSync = true;

		//Initializing GLFW
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		//Create Window
		window = glfwCreateWindow(scr_width, scr_height, "Strikezone", NULL, NULL);
		if (window == NULL)
		{
			std::cout << "[GameEngine] Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return false;
		}
		glfwMakeContextCurrent(window);
		glfwGetWindowPos(window, &video.windowedPos.x, &video.windowedPos.y);

		//Init GLAD
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "[GameEngine] Failed to initialize GLAD" << std::endl;
			return false;
		}

		//Viewport and Callbacks + Setting GLFW pointer to this core
		glViewport(0, 0, scr_width, scr_height);

		glfwSetErrorCallback(GLFWErrorCallback);
		glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
		glfwSetKeyCallback(window, KeyCallback);
		glfwSetMouseButtonCallback(window, MouseButtonCallback);
		glfwSetScrollCallback(window, ScrollCallback);
		glfwSetWindowFocusCallback(window, WindowFocusCallback);
		glfwSetCursorPosCallback(window, CursorPosCallback);
		glfwSetWindowPosCallback(window, WindowPosCallback);
		glfwSetWindowUserPointer(window, this);
		if (glfwRawMouseMotionSupported()) { glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); }

		video.windowFocused = glfwGetWindowAttrib(window, GLFW_FOCUSED);
		//ToggleFullscreen();
		RefreshDisplayDims();

		//Screen Polygon
		Safe_GlGenVertexArrays(1, &screenVAO);
		Safe_GlGenBuffers(1, &screenVBO);

		glBindVertexArray(screenVAO);
		glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

		std::vector<float> quad_verts
		{
		   -1.0f, 1.0f,		0,  1,
			1.0f, 1.0f,		1,  1,
			1.0f,-1.0f,		1,  0,
			1.0f,-1.0f,		1,  0,
		   -1.0f,-1.0f,		0,  0,
		   -1.0f, 1.0f,		0,  1
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * quad_verts.size(), quad_verts.data(), GL_STATIC_DRAW);

		//Initializing Assets
		assets = std::make_unique<AssetManager>();
		LoadContent();

		//Initializing Scope Stack
		PushScope(std::make_unique<GameScope>());
		PushScope(std::make_unique<StartMenu>(*this));
		//cursorCanvas = std::make_unique<UICanvas>();

		return true;

	}

	void GameEngine::LoadContent()
	{
		assets->LoadAssets();
	}

	void GameEngine::PushScope(std::unique_ptr<GameScope> scope)
	{
		GameScope* burried = nullptr;
		if (!scopeStack.empty()) burried = scopeStack.top().get();
		scopeStack.push(std::move(scope));
		//SimulationRenewed() inlined here
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}
		if (burried) burried->NotifyBurried();
	}

	void GameEngine::PopScope()
	{
		if (scopeStack.size() > 1)
		{
			scopeStack.top()->Terminate();	scopeStack.pop();
			scopeStack.top()->ResetRenderTarget();
			scopeStack.top()->NotifySurfaced();
			GMInput.SetShowCursor(true);
		}

		if (scopeStack.size() <= 1)
		{
			glfwSetWindowShouldClose(window, true);
		}
		//running = false;
	}
	//
	void GameEngine::ProcessInput()
	{
		glfwPollEvents();

		GMInput.UpdateInputStates();
		scopeStack.top()->ProcessInput();
	}

	void GameEngine::Update()
	{
		assets->audio.UpdateSourcesAndTracks(*this);
		glfwSwapInterval(video.vSync);

		auto& topScope = scopeStack.top();
		auto stackCount = scopeStack.size();
		glfwSetInputMode(window, GLFW_CURSOR, GMInput.showCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_DISABLED);
		video.windowHovered = glfwGetWindowAttrib(window, GLFW_HOVERED) == GLFW_TRUE;
		if (!topScope->_template)
		{
			topScope->PackUI();
			if (scopeStack.size() >= stackCount) topScope->Update();
		}
		
		//Extra...
		GMInput.mouseScroll = 0;
	}

	void GameEngine::Render()
	{
		glPixelStorei(GL_PACK_ALIGNMENT, 1); //just a little thing 

		//Render Current Scope's Content
		auto& topScope = scopeStack.top();
		topScope->Render();

		//Draw on the Window
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, video.screenDims.x, video.screenDims.y);

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		
		auto& shader = assets->engineScreenVF;
		if (shader.GlUseShader())
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glUniform4fv(shader.GetUniformLocation("scrDisplaySize"), 1, glm::value_ptr(glm::vec4(video.screenDims, video.displayDims)));

			topScope->renderBfr.GlBindTexture(0);
			glUniform1i(shader.GetUniformLocation("aTexture", false), 0);

			topScope->renderBfr.GlBindDepthTexture(1);
			glUniform1i(shader.GetUniformLocation("aDepth", true), 1);

			RenderQuad();
			glBindTexture(GL_TEXTURE_2D, 0);

			//cursor
			auto& uiShader = assets->uiShader;
			if (video.windowFocused && video.windowHovered && uiShader.GlUseShader())
			{
				glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(GMInput.curMPos, 0));
				model = glm::scale(model, glm::vec3(1.5f));
				glm::mat4 transform = glm::ortho(0.f, (float)video.screenDims.x, (float)video.screenDims.y, 0.f, -100.f, 100.f);
				transform = transform * model;

				//transform = glm::translat(glm::mat4(1), glm::vec3(1.f, 0.f, 0.f));

				uiShader.SetSamplerUniform("atlas", assets->uiatlas, 0, false);
				uiShader.SetUniform("transform", transform);
				uiShader.SetUniform("uSpritesPerRow", assets->uiatlas.GetCapacity());
				uiShader.SetUniform("spriteUnit", assets->uiatlas.GetUnit());

				glBindVertexArray(assets->cursorVAO);
				glDrawArrays(GL_TRIANGLES, 0, 18);
				glBindVertexArray(0);
			}
		}

		///////////		
		//Cursor
		/*
		if (GMInput.showCursor)
		{
			auto color = glm::vec4(1);
			const auto& spriteRect = assets->uiatlas.GetSprite(3);
			float tileIndex = spriteRect.X + assets->uiatlas.GetCapacity() * spriteRect.Y;
			double xpos, ypos; glfwGetCursorPos(window, &xpos, &ypos);	glm::vec2 toScrOrigin(xpos, ypos);
			glm::vec2 toScrArea(8 * 4);
			glm::vec2 texBL(0, -8);

			if (fullscreen)
			{
				if (t_adjustToWindowDims)
				{
					toScrOrigin /= t_windowDimsDivider;
					toScrArea /= t_windowDimsDivider;
				}
				else
				{
					auto multiplier = glm::vec2(dispDims) / glm::vec2(width, height);
					toScrOrigin *= multiplier;
					toScrArea *= multiplier;
				}
			}

			std::vector<UIB::UIVert> curVerts = {
				 {{toScrOrigin.x, toScrOrigin.y}, {texBL.x, texBL.y - 8}, tileIndex, color},
				 {{toScrOrigin.x + toScrArea.x, toScrOrigin.y}, {texBL.x + 8, texBL.y - 8}, tileIndex, color} ,
				 {{toScrOrigin.x + toScrArea.x, toScrOrigin.y + toScrArea.y}, {texBL.x + 8, texBL.y}, tileIndex, color} ,
				 {{toScrOrigin.x, toScrOrigin.y + toScrArea.y}, {texBL.x, texBL.y}, tileIndex, color}
			};
			vector<uint32_t> curInds = { 0,1,2,0,2,3 };

			cursorCanvas->AppendUIVertexData(curVerts, curInds);
			cursorCanvas->RenderUIContent(*this);
		}
		*/
		glfwSwapBuffers(window);
	}

	//
	void GameEngine::SetDisplayDims(glm::ivec2 dims)
	{
		if (video.displayDims == dims) return;
		video.displayDims = dims;
		if (scopeStack.size() > 0)
		{
			if (scopeStack.top().get()) scopeStack.top()->ResetRenderTarget();
		}
	}

	void GameEngine::ToggleFullscreen()
	{
		video.fullscreen = !video.fullscreen;

		if (video.fullscreen) {
			// Save old windowed size/pos
			glfwGetWindowSize(window, &video.windowedDims.x, &video.windowedDims.y);
			glfwGetWindowPos(window, &video.windowedPos.x, &video.windowedPos.y);
		}
		RefreshDisplayDims();
	}

	void GameEngine::RefreshDisplayDims()
	{
		if (video.fullscreen)
		{
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			video.IGNORE_WIN_SIZE_CHANGES = true;
			glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
			glfwSetWindowMonitor(window, nullptr, 0, 0, mode->width, mode->height, 0);
			video.IGNORE_WIN_SIZE_CHANGES = false;
			glfwGetWindowSize(window, &video.screenDims.x, &video.screenDims.y);
			if (video.fillMonitorDims)
			{
				SetDisplayDims(video.screenDims);// / t_windowDimsDivider);
			}
			else
			{
				float r = (float)video.screenRatio.x / (float)video.screenRatio.y;
				glm::vec2 scrDims = video.screenDims;
				SetDisplayDims(r > scrDims.x / scrDims.y ? glm::ivec2(scrDims.x, scrDims.x / r) : glm::ivec2(scrDims.y * r, scrDims.y));
			}
		}
		else
		{
			if (video.windowedDims == glm::ivec2(0)) return;
			glm::ivec2 scrPos = video.windowedPos;

			video.IGNORE_WIN_SIZE_CHANGES = true;
			glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
			glfwSetWindowMonitor(window, nullptr, scrPos.x, scrPos.y, video.windowedDims.x, video.windowedDims.y, 0);
			video.IGNORE_WIN_SIZE_CHANGES = false;
			glfwGetWindowSize(window, &video.screenDims.x, &video.screenDims.y);

			SetDisplayDims(video.windowedDims);
		}
	}
	
	//
	double GameEngine::HandleClock(double& lastTime) const
	{
		double currentTime = glfwGetTime();
		double elapsedTime = currentTime - lastTime;

		if (video.targetFPS > 0)
		{
			double frameTime = 1.0 / video.targetFPS;

			// 1. Sleep for the bulk of the remaining time to save CPU cycles
			// Leaving 0.0015 seconds (1.5ms) as a buffer for OS scheduling jitter
			double remainingTime = frameTime - elapsedTime;
			if (remainingTime > 0.0015)
			{
				std::this_thread::sleep_for(std::chrono::duration<double>(remainingTime - 0.0015));
			}

			// 2. Precise busy-wait (spin) for the final sub-millisecond precision
			while (currentTime = glfwGetTime(), elapsedTime = currentTime - lastTime, elapsedTime < frameTime)
			{
				std::this_thread::yield(); // Prevents 100% core starvation during the tiny spin window
			}
		}
		else
		{
			currentTime = glfwGetTime();
			elapsedTime = currentTime - lastTime;
		}

		lastTime = currentTime;
		return elapsedTime;
	}


}