#pragma once

#include "SafeGL.hpp"
#include "UIWidget.hpp"
#include "Assets.hpp"
#include "Controls.hpp"

namespace Engine { struct VideoMode; class GameEngine; }
using namespace SGL;
namespace UIB
{
	class UICanvas;
	// - UIContainer -
	class UIContainer //Parent for HUD, Window and Modals
	{
	protected:
		Anchor _anchor;
		std::unordered_map<int, std::unique_ptr<Widget>> _widgets;

	public:
		UICanvas* _canvas;

		UIContainer(UICanvas& canvas);
		virtual void TakeInput(const TLB::GameInput& input, glm::ivec2 dispDims, bool& mouseCaptured, bool& keyboardCaptured) {}
		virtual void DrawToCanvas(const Engine::VideoMode& video, const glm::mat4& Gproj, float zUnit) {}
		virtual void GotCalled() {}
		virtual glm::vec2 GetRelativePosAt(glm::ivec2 pos) const;
	};

	// - HUD -
	class HUD : public UIContainer
	{
	public:
		HUD(UICanvas& canvas) : UIContainer(canvas) {}
	};

	// - Window -
	class Window : public UIContainer
	{
	public:
		glm::ivec2 size;
		bool beingDragged = false;
		glm::ivec2 dragStartMPos = { 0, 0 };
		bool called = true, collapsed = false;
		std::string title;
		glm::ivec2 nextLine;
		//fields for: resizable, draggable

		Window(UICanvas& canvas, glm::ivec2 winPos, glm::ivec2 winSize) : UIContainer(canvas), size(winSize)
		{
			_anchor.pxlOffset = winPos;
		}

		void DrawToCanvas(const Engine::VideoMode& video, const glm::mat4& Gproj, float zUnit) override;
		void TakeInput(const TLB::GameInput& input, glm::ivec2 dispDims, bool& mouseCaptured, bool& keyboardCaptured) override;
		void GotCalled() override;
		glm::vec2 GetRelativePosAt(glm::ivec2 pos) const override;

		// Calling Widgets
		std::pair<Button*, bool> CallButton(int id);
	};

	// - UICanvas -
	class UICanvas
	{
		std::unique_ptr<HUD> _hud;
		std::unordered_map<int, std::unique_ptr<Window>> _windows;
		vector<Window*> focusWins;
		bool midFrame = false;

	public:
		Engine::GameEngine* engine;
		TLB::AssetManager* assets;
		SmartGLID uiVAO, uiVBO, uiEBO;
		SmartGLID textVAO, textVBO;

		UICanvas(Engine::GameEngine& Engine);
		void DispatchInput();
		void RenderContent(const Engine::VideoMode& video);
		void SurfaceWindow(int index);
		void SurfaceWindowId(int id);
		void DrawPanel(const glm::mat4& space, glm::vec2 size) const;
		void DrawSeperator(const glm::mat4& space, glm::vec2 center, int length) const;

		void StartFrame();
		void EndFrame();
		std::pair<Window*, bool> CallWindow(int id);
		void ForgetWindow(int id);
		
	};
}