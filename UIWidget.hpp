#pragma once

#include "Controls.hpp"

namespace UIB
{
	// Measurements
	inline constexpr int QC = 8;	 // Quarter-corner
	inline constexpr int HC = 2 * QC;// Half-corner
	inline constexpr int FC = 2 * HC;// Full-corner
	//
	class UICanvas;
	class Window;

	// - UIVert -
	struct UIVert
	{
		glm::vec3 pos;
		float spriteIndex;
		glm::vec2 uvs;
		glm::vec4 color = glm::vec4(1);
	};

	// - Anchor -
	struct Anchor
	{
		glm::vec2 scrOffset;
		glm::ivec2 pxlOffset;
		float scale;

		Anchor(glm::vec2 screenRatioOffset, glm::ivec2 pixelOffset, float uiscale = 1.f) :
			scrOffset(screenRatioOffset), pxlOffset(pixelOffset), scale(uiscale)
		{}
	};

	// - WidgetType -
	enum class WidgetType : uint8_t
	{
		Button
	};

	// - Widget -
	class Widget //Parent for Buttons, Checkboxes, Fields, etc....
	{
	public:
		Window* _win;
		glm::ivec2 _pos;
		bool called = true;

		Widget(Window& win) : _win(&win) {}
		virtual void ConsumeInput(const TLB::GameInput& input, bool& mouseConsumed) {}
		virtual void Called(glm::ivec2 pos) { called = true; _pos = pos; }
		virtual void Draw(const glm::mat4& space) {}

	};

	// - Button -
	class Button : public Widget
	{
	public:
		bool hovered = false, pressed = false, clicked = false;
		std::string title;
		int length = 2 * FC;

		Button(Window& win);
		void ConsumeInput(const TLB::GameInput& input, bool& mouseConsumed) override;
		void Called(glm::ivec2 pos) override;
		void Draw(const glm::mat4& space) override;
	};
}