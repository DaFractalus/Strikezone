#include "UIkit.hpp"
#include "UIWidget.hpp"
#include "GameEngine.hpp"
#include "Font.hpp"

namespace UIB
{
	// --- UIContainer ---
	UIContainer::UIContainer(UICanvas& canvas) : _canvas(&canvas), _anchor(glm::vec2(0.f), glm::ivec2(0)) {}

	glm::vec2 UIContainer::GetRelativePosAt(glm::ivec2 pos) const
	{
		glm::vec2 dispDims = _canvas->engine->Video().displayDims;
		glm::vec2 relPos = glm::vec2(pos) - _anchor.scrOffset * dispDims; // relative to scrOffset
		relPos = relPos / _anchor.scale - glm::vec2(_anchor.pxlOffset); // scaled down + relative to pxlOffset
		return relPos;
	}

	// --- Window ---
	void Window::DrawToCanvas(const Engine::VideoMode& video, const glm::mat4& Gproj, float zUnit)
	{
		auto& assets = *_canvas->assets;
		nextLine = glm::ivec2(0);

		glm::mat4 anchoring = glm::translate(glm::mat4(1), glm::vec3(_anchor.scrOffset * glm::vec2(video.displayDims), zUnit));
		anchoring = glm::scale(anchoring, glm::vec3(_anchor.scale));
		anchoring = glm::translate(anchoring, glm::vec3(_anchor.pxlOffset, 0));

		glm::mat4 space = Gproj * anchoring;
		_canvas->DrawPanel(space, size);

		auto& uiShader = assets.uiShader;
		if (!uiShader.GlUseShader()) return;

		auto& rect = assets.uiatlas.GetSprite(-2);
		float tileIndex = rect.X + assets.uiatlas.GetCapacity() * rect.Y;

		glm::mat4 transform = space;

		uiShader.SetSamplerUniform("atlas", assets.uiatlas, 0, false);
		uiShader.SetUniform("transform", transform);
		uiShader.SetUniform("uSpritesPerRow", assets.uiatlas.GetCapacity());
		uiShader.SetUniform("spriteUnit", assets.uiatlas.GetUnit());

		// Highlight
		glm::vec4 highlightCol(1.f, 0.7f, 0.f, 1);
		int hlExtend = size.x /3;
		vector<UIVert> verts1 = {
			{glm::vec3(HC, 0, 0), tileIndex, glm::vec2(0), highlightCol},
			{glm::vec3(0, FC, 0), tileIndex, glm::vec2(0), highlightCol},
			{glm::vec3(0, HC, 0), tileIndex, glm::vec2(0), highlightCol},
			{glm::vec3(FC, 0, 0), tileIndex, glm::vec2(0), highlightCol},

			{glm::vec3(FC, 0, 0), tileIndex, glm::vec2(0), highlightCol},
			{glm::vec3(FC  + hlExtend - 2, 4, 0), tileIndex, glm::vec2(0), highlightCol},

			{glm::vec3(FC + hlExtend , 0, 0), tileIndex, glm::vec2(0), highlightCol},
			{glm::vec3(FC - 4, 4, 0), tileIndex, glm::vec2(0), highlightCol},
		};

		vector<uint> inds1 = { 0, 1, 2, 0, 1, 3, 4, 5, 6, 4, 5, 7 };

		inds1.reserve(inds1.size() * 2);
		for (auto& i : inds1)
		{
			inds1.push_back(i + verts1.size());
		}

		verts1.reserve(verts1.size() * 2);
		for(auto& v : verts1)
		{
			auto newV = v;
			newV.pos = glm::vec3(size + glm::ivec2(2), 0) - newV.pos;
			v.pos -= glm::ivec3(2, 2, 0);
			verts1.push_back(newV);
		}

		_canvas->uiVBO.uploadBufferData(verts1, GL_ARRAY_BUFFER, GL_STATIC_DRAW);
		_canvas->uiEBO.uploadBufferData(inds1, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);

		glBindVertexArray(_canvas->uiVAO);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(inds1.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// Title
		glm::vec2 titlePos(FC, HC);
		glm::vec4 textCol(1);
		auto textVerts = assets.uifont.RenderText(title, titlePos, 24, glm::vec2(0.f, 0.5f), textCol, textCol, textCol, textCol, nullptr);

		auto& textShader = assets.textShader;
		if (textShader.GlUseShader())
		{
			textShader.SetUniform("transform", space);
			textShader.SetSamplerUniform("textfont", assets.uifont, 0, false);

			_canvas->textVBO.uploadBufferData(textVerts, GL_ARRAY_BUFFER, GL_STATIC_DRAW);

			if (textVerts.size() > 0)
			{
				glBindVertexArray(_canvas->textVAO);
				glDrawArrays(GL_TRIANGLES, 0, textVerts.size());
				glBindVertexArray(0);
			}
			glBindTexture(GL_TEXTURE_2D, 0);
			textVerts.clear();
		}

		//Title Seperator
		glm::vec2 sepCenter(size.x / 2.f, FC + QC);
		_canvas->DrawSeperator(space, sepCenter, size.x - HC);

		// Widgets
		glm::mat4 widgetSpace = space * glm::translate(glm::mat4(1), glm::vec3(QC, FC + QC + 4, 0));
		for (auto& pair : _widgets)
		{
			pair.second->Draw(widgetSpace);
		}
	}

	void Window::TakeInput(const TLB::GameInput& input, glm::ivec2 dispDims, bool& mouseCaptured, bool& keyboardCaptured)
	{
		//Throught process dump:
		//if mouse just got captured and if mouse cur pressed on the titlebar ( and was not prev pressed), store that as "being dragged" internally
		//if being dragged is true and mouse cur pressed, whether captured or not, DO move the window with the mouse and consider the mouse captured
		//if mouse cur isnt pressed, being dragged gets set to false

		// check input's mousecaptured, if it's false we do the following:
		//first check if being dragged, if yes check if cur pressed, if yes move the window and set a local bool "titlebarHovered" to true
		//titlebarHovered then confirms if not yet true, by checking mouse pos if its on window's titlebar
		// mousecaptured becomes true if titlebarHovered is true or mouse is on the window
		// if mousecatured is true, we continue to check for mouse input

		//keyboard will work by indexing or storing the pointer to an active widget, if no widget is actively using the keyboard then keyboardcaptured stays false

		//first, check if mouse input is alreadt consumed, if yes then infornt the widgets
		if (input.MouseCaptured)
		{
			//check mouse clicks
			bool temp = false;
			for (auto& pair : _widgets)
			{
				pair.second->ConsumeInput(input, temp);
			}
			return;
		}



		glm::vec2 winCorner = glm::vec2(_anchor.pxlOffset) * _anchor.scale + glm::vec2(dispDims) * _anchor.scrOffset;
		bool titlebarHovered = false;
		if (beingDragged)
		{
			if (input.CurMouseButton(GLFW_MOUSE_BUTTON_LEFT))
			{
				_anchor.pxlOffset = glm::ivec2(input.curMPos) - dragStartMPos;
				titlebarHovered = true;
			}
			else beingDragged = false;
		}
		else
		{
			glm::vec2 tbTopright = winCorner + glm::vec2(size.x, FC) * _anchor.scale;
			auto mpos = input.curMPos;
			titlebarHovered = winCorner.x < mpos.x && mpos.x < tbTopright.x && winCorner.y < mpos.y && mpos.y < tbTopright.y;
			if (titlebarHovered && input.CurMouseButton(GLFW_MOUSE_BUTTON_LEFT) && !input.PrevMouseButton(GLFW_MOUSE_BUTTON_LEFT))
			{
				dragStartMPos = mpos - winCorner;
				beingDragged = true;
			}

		}

		if (titlebarHovered)
		{
			//check closing or any button in title bar
			mouseCaptured = true;
		}
		else
		{
			glm::vec2 winBotRight = winCorner + glm::vec2(size) * _anchor.scale;
			auto mpos = input.curMPos;
			mouseCaptured = winCorner.x < mpos.x && mpos.x < winBotRight.x && winCorner.y < mpos.y && mpos.y < winBotRight.y;
			if (mouseCaptured)
			{
				//check mouse clicks
				for (auto& pair : _widgets)
				{
					bool mCons = false;
					pair.second->ConsumeInput(input, mCons);
				}
			}
		}
	}

	void Window::GotCalled()
	{
		called = true;
		for (auto& pair : _widgets) pair.second->called = false;
	}

	glm::vec2 Window::GetRelativePosAt(glm::ivec2 pos) const
	{
		glm::vec2 relPos = UIContainer::GetRelativePosAt(pos) - glm::vec2(QC, FC + QC + 4);
		return relPos;
	}

	std::pair<Button*, bool> Window::CallButton(int id)
	{
		bool existed = _widgets.contains(id);
		if (!existed)_widgets.try_emplace(id, std::make_unique<Button>(*this));

		UIB::Button* btn = static_cast<Button*>(_widgets.at(id).get());
		btn->Called(nextLine);
		nextLine += glm::ivec2(0, FC + 4);

		return { btn, existed };
	}

	// --- UICanvas ---
	UICanvas::UICanvas(Engine::GameEngine& Engine) : engine(&Engine), assets(Engine.assets.get()), uiVAO(GLObjectType::VertexArray),
		uiVBO(GLObjectType::Buffer), uiEBO(GLObjectType::Buffer), textVAO(GLObjectType::VertexArray),
		textVBO(GLObjectType::Buffer)
	{
		uiVAO.setVAOLayout([this]()
			{
				glBindVertexArray(uiVAO);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiEBO);
				glBindBuffer(GL_ARRAY_BUFFER, uiVBO);

				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(UIVert), (void*)0);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(UIVert), (void*)offsetof(UIVert, uvs));
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(UIVert), (void*)offsetof(UIVert, color));

				glBindVertexArray(0);
			});

		textVAO.setVAOLayout([this]()
			{
				glBindVertexArray(textVAO);
				glBindBuffer(GL_ARRAY_BUFFER, textVBO);

				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TextVert), (void*)0);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVert), (void*)offsetof(TextVert, tex));
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(TextVert), (void*)offsetof(TextVert, col));

				glBindVertexArray(0);
			});
	}

	void UICanvas::DispatchInput()
	{
		auto& input = engine->Input();
		glm::ivec2 dispDims = engine->Video().displayDims;
		bool& globalMouseCap = input.MouseCaptured;
		bool& globalKeyboardCap = input.KeyboardCaptured;
		int index = 0;
		for (auto& win : focusWins)
		{
			//if (globalKeyboardCap && globalMouseCap) break;
			if (!win->called) { index++; continue; }

			bool mCap = false, kCap = false;
			win->TakeInput(input, dispDims, mCap, kCap);

			if (mCap && !globalMouseCap)
			{
				if (index > 0 && input.CurMouseButton(GLFW_MOUSE_BUTTON_LEFT)
					&& !input.PrevMouseButton(GLFW_MOUSE_BUTTON_LEFT))
				{
					SurfaceWindow(index);
				}
			}
			globalMouseCap |= mCap; globalKeyboardCap |= kCap;
			index++;
		}

		if (_hud && !(globalKeyboardCap && globalMouseCap))
		{
			bool hudMCap = false, hudKCap = false;
			_hud->TakeInput(input, dispDims, hudMCap, hudKCap);
		}
	}

	void UICanvas::SurfaceWindow(int index)
	{
		if (index >= focusWins.size()) return;
		std::rotate(focusWins.begin(), focusWins.begin() + index, focusWins.begin() + index + 1);
	}

	void UICanvas::SurfaceWindowId(int id)
	{
		if (!_windows.contains(id)) return;
		auto ptr = _windows.at(id).get();

		auto it = std::find(focusWins.begin(), focusWins.end(), ptr);
		if (it == focusWins.end()) return;

		int index = std::distance(focusWins.begin(), it);
		std::rotate(focusWins.begin(), focusWins.begin() + index, focusWins.begin() + index + 1);
	}

	void UICanvas::DrawPanel(const glm::mat4& space, glm::vec2 size) const
	{
		glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(-2, -2, 0));
		model = glm::scale(model, glm::vec3(size + glm::vec2(4), 1));
		model = glm::scale(model, glm::vec3(.5f, .5f, 1));
		model = glm::translate(model, glm::vec3(1.f, 1.f, 0.f));
		glm::mat4 transform = space * model;

		glm::vec4 insideCol = glm::vec4(20, 30, 25, 255) / 255.f;
		glm::vec4 borderCol = glm::vec4(90, 100, 110, 255) / 255.f;

		auto& panelShader = assets->panelShader;
		panelShader.GlUseShader();
		panelShader.SetUniform("transform", transform);
		panelShader.SetUniform("panelDims", size);
		panelShader.SetUniform("insideCol", insideCol);
		panelShader.SetUniform("borderCol", borderCol);
		engine->RenderQuad();
	}

	void UICanvas::DrawSeperator(const glm::mat4& space, glm::vec2 center, int length) const
	{
		glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(center, 0));
		glm::mat4 transform = space * model;

		glm::vec4 color = glm::vec4(90, 100, 110, 255) / 255.f;

		auto& uisepShader = assets->uisepShader;
		uisepShader.GlUseShader();
		uisepShader.SetUniform("transform", transform);
		uisepShader.SetUniform("lineLength", (float)length);
		uisepShader.SetUniform("color", color);
;
		glBindVertexArray(uiVAO);
		glLineWidth(1);
		glDrawArrays(GL_LINES, 0, 2);
		glBindVertexArray(0);
	}

	void UICanvas::RenderContent(const Engine::VideoMode& video)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);

		float maxZ = static_cast<float>(_windows.size()) + 1.f; //each container gets a range of 1 to order its components by Z
		glm::mat4 proj = glm::ortho(0.f, (float)video.displayDims.x, (float)video.displayDims.y, 0.f, 0.f, -maxZ);

		if (_hud) _hud->DrawToCanvas(video, proj, 0);//Drawing HUD
		float currentZ = maxZ;
		for (auto it = focusWins.rbegin(); it != focusWins.rend(); ++it)
			//for (auto& win : _windows)//Drawing Windows
		{
			currentZ--;
			auto win = *it;
			if (!win->called) continue;
			win->DrawToCanvas(video, proj, currentZ);
		}
	}

	void UICanvas::StartFrame()
	{
		assert(!midFrame && "Attempted starting a UICanvas frame without ending the active one.");
		midFrame = true;
		for (auto& pair : _windows)
		{
			pair.second->called = false;
		}
	}

	void UICanvas::EndFrame()
	{
		assert(midFrame && "Attempted ending a UICanvas frame without starting one.");
		midFrame = false;

	}

	std::pair<Window*, bool> UICanvas::CallWindow(int id)
	{
		assert(midFrame && "Attempted creating a window without starting a frame.");
		Window* win;
		if (_windows.contains(id))
		{
			win = _windows.at(id).get();
			win->GotCalled();
			return { win, false };
		}
		else
		{
			_windows.try_emplace(id, std::make_unique<Window>(*this, glm::ivec2(200), glm::ivec2(200, 300)));
			win = _windows.at(id).get();
			focusWins.insert(focusWins.begin(), win);
			return { win, true };
		}
	}

	void UICanvas::ForgetWindow(int id)
	{
		if (!_windows.contains(id)) return;

		auto win = _windows.at(id).get();
		std::erase(focusWins, win);
		_windows.erase(id);
	}
}