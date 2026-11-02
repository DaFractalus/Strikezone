#include "UIWidget.hpp"
#include "UIkit.hpp"

namespace UIB
{
	// --- Button ---
	Button::Button(Window& win) : Widget(win) {}

	void Button::ConsumeInput(const TLB::GameInput& input, bool& mouseConsumed)
	{
		if (input.MouseCaptured) { pressed = false; hovered = false; clicked = false; return; }

		glm::vec2 winMPos = _win->GetRelativePosAt(input.curMPos);
		auto relMPos = winMPos - glm::vec2(_pos);

		hovered = (0 <= relMPos.x && relMPos.x < length && 0 <= relMPos.y && relMPos.y < FC);
		mouseConsumed = hovered;
		clicked = false;

		if (hovered)
		{
			if (input.CurMouseButton(GLFW_MOUSE_BUTTON_LEFT))
			{
				pressed = pressed || !input.PrevMouseButton(GLFW_MOUSE_BUTTON_LEFT);
			}
			else if (pressed) { clicked = true; pressed = false; }
		}
		else pressed = false;
		
	}

	void Button::Called(glm::ivec2 pos)
	{
		Widget::Called(pos);
	}

	void Button::Draw(const glm::mat4& space)
	{
		auto& canvas = *_win->_canvas;
		auto& assets = *canvas.assets;
		auto& rect = assets.uiatlas.GetSprite(-2);
		float tileIndex = rect.X + assets.uiatlas.GetCapacity() * rect.Y;

		// Button
		auto& uiShader = assets.uiShader;
		if (uiShader.GlUseShader())
		{
			glm::vec4 buttonCol = hovered ? glm::vec4(1.f, 0.7f, 0.f, 1) : glm::vec4(50, 55, 52, 255) /255.f;
			int indent = QC;
			vector<UIVert> verts = {
				{glm::vec3(indent, 0, 0), tileIndex, glm::vec2(0), buttonCol},
				{glm::vec3(length - indent, FC, 0), tileIndex, glm::vec2(0), buttonCol},
				{glm::vec3(length - indent, 0, 0), tileIndex, glm::vec2(0), buttonCol},
				{glm::vec3(indent, FC, 0), tileIndex, glm::vec2(0), buttonCol},

				{glm::vec3(0, FC, 0), tileIndex, glm::vec2(0), buttonCol},
				{glm::vec3(0, indent, 0), tileIndex, glm::vec2(0), buttonCol},
				{glm::vec3(length, 0, 0), tileIndex, glm::vec2(0), buttonCol},
				{glm::vec3(length, FC - indent, 0), tileIndex, glm::vec2(0), buttonCol},
			};

			vector<uint> inds = {0, 1, 2, 0, 1, 3, 0, 3, 5, 3, 4, 5, 1, 2, 7, 2, 6, 7};

			glm::mat4 transform = space * glm::translate(glm::mat4(1), glm::vec3(_pos, 0));

			uiShader.SetSamplerUniform("atlas", assets.uiatlas, 0, false);
			uiShader.SetUniform("transform", transform);
			uiShader.SetUniform("uSpritesPerRow", assets.uiatlas.GetCapacity());
			uiShader.SetUniform("spriteUnit", assets.uiatlas.GetUnit());

			canvas.uiVBO.uploadBufferData(verts, GL_ARRAY_BUFFER, GL_STATIC_DRAW);
			canvas.uiEBO.uploadBufferData(inds, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);

			glBindVertexArray(canvas.uiVAO);
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(inds.size()), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
		// Title
		glm::vec2 titlePos((float)length/2.f, HC);
		titlePos += _pos;
		glm::vec4 textCol = (pressed || !hovered) ? glm::vec4(1) : glm::vec4(0, 0, 0, 1);
		auto textVerts = assets.uifont.RenderText(title, titlePos, 16, glm::vec2(0.5f, 0.5f), textCol, textCol, textCol, textCol, nullptr);

		auto& textShader = assets.textShader;
		if (textShader.GlUseShader())
		{
			textShader.SetUniform("transform", space);
			textShader.SetSamplerUniform("textfont", assets.uifont, 0, false);

			canvas.textVBO.uploadBufferData(textVerts, GL_ARRAY_BUFFER, GL_STATIC_DRAW);

			if (textVerts.size() > 0)
			{
				glBindVertexArray(canvas.textVAO);
				glDrawArrays(GL_TRIANGLES, 0, textVerts.size());
				glBindVertexArray(0);
			}
			glBindTexture(GL_TEXTURE_2D, 0);
			textVerts.clear();
		}
	}
}