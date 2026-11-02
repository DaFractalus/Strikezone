#include "Controls.hpp"
#include "GameEngine.hpp"

namespace TLB
{
    GameInput::GameInput(Engine::GameEngine& game) : game(&game), prevMBL(false), prevMBM(false), prevMBR(false), curMBL(false), curMBM(false), curMBR(false),
		mouseScroll(0), curMPos(0), prevMPos(0), KeyboardCaptured(false), MouseCaptured(false), showCursor(true), showCursorToggled(true)
    {
        kbInputTable[KeyBind::Forward] = GLFW_KEY_W;
        kbInputTable[KeyBind::Backward] = GLFW_KEY_S;
        kbInputTable[KeyBind::Left] = GLFW_KEY_A;
        kbInputTable[KeyBind::Right] = GLFW_KEY_D;
        kbInputTable[KeyBind::Jump] = GLFW_KEY_SPACE;
        kbInputTable[KeyBind::Sneak] = GLFW_KEY_LEFT_SHIFT;
        kbInputTable[KeyBind::Sprint] = GLFW_KEY_LEFT_CONTROL;
        kbInputTable[KeyBind::Pause] = GLFW_KEY_P;
        kbInputTable[KeyBind::ToggleFullscreen] = GLFW_KEY_F11;
        kbInputTable[KeyBind::OpenInventory] = GLFW_KEY_F;
        kbInputTable[KeyBind::ReturnBack] = GLFW_KEY_ESCAPE;
        kbInputTable[KeyBind::ToggleDebug] = GLFW_KEY_F3;
        kbInputTable[KeyBind::LowerFOV] = GLFW_KEY_EQUAL;
        kbInputTable[KeyBind::RaiseFOV] = GLFW_KEY_MINUS;
        //kbInputTable["OpenChat"] = (int)Keys.Enter;
        //kbInputTable["ScrollUp"] = (int)Keys.OemCloseBrackets;
        //kbInputTable["ScrollDown"] = (int)Keys.OemOpenBrackets;
        //kbInputTable["PickupItems"] = (int)Keys.Q;
    }

    bool GameInput::CurMouseButton(int button, bool forced) const
    {

        if (MouseCaptured && !forced) return false;

        switch (button)
        {
        case GLFW_MOUSE_BUTTON_LEFT: return curMBL;
        case GLFW_MOUSE_BUTTON_MIDDLE: return curMBM;
        case GLFW_MOUSE_BUTTON_RIGHT: return curMBR;
        default: return false;
        }
    }

    bool GameInput::PrevMouseButton(int button, bool forced) const
    {
        if (MouseCaptured && !forced) return false;
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_LEFT: return prevMBL;
        case GLFW_MOUSE_BUTTON_MIDDLE: return prevMBM;
        case GLFW_MOUSE_BUTTON_RIGHT: return prevMBR;
        default: return false;
        }
    }

    bool GameInput::IsCurrentKeyPressed(KeyBind keyBind, bool forced) const
    {
        if ((KeyboardCaptured && !forced) ? false : kbInputTable.find(keyBind) != kbInputTable.end())
        {
            return glfwGetKey(game->window, kbInputTable.at(keyBind)) == GLFW_PRESS;
        }
        return false;
    }

    bool GameInput::WasKeyClicked(KeyBind keyBind, int event_key, int action, bool force) const
    {
        if (!force && KeyboardCaptured) return false;
        return action == GLFW_PRESS && kbInputTable.at(keyBind) == event_key;
    }

    bool GameInput::WasMButtonClicked(int button, int event_button, int action, bool force) const
    {
        if (!force && MouseCaptured) return false;
        return action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT;
    }

    void GameInput::UpdateInputStates()
    {
        bool inAllow = game->Video().windowFocused;
        prevMBL = curMBL;
        prevMBM = curMBM;
        prevMBR = curMBR;
        curMBL = inAllow && glfwGetMouseButton(game->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        curMBM = inAllow && glfwGetMouseButton(game->window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
        curMBR = inAllow && glfwGetMouseButton(game->window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

        MouseCaptured = !(game->Video().windowHovered && game->Video().windowFocused);// ImGui::GetIO().WantCaptureMouse; //TODO: get state from UI, or something idk
        KeyboardCaptured = false;// ImGui::GetIO().WantCaptureKeyboard; //TODO: same here
    }

    void GameInput::UpdateCursorPos(double xpos, double ypos)
    {
        if (showCursor)
        {
            prevMPos = curMPos;
            if (game->Video().windowFocused)
            {
                curMPos = glm::vec2(xpos, ypos);
                if (game->Video().fullscreen)
                {
                    if (game->Video().fillMonitorDims) curMPos /= 1;// game->Video().t_windowDimsDivider;
                    else
                    {
                        curMPos -= glm::vec2(game->Video().screenDims - game->Video().displayDims) / 2.f;
                    }
                }
            }
            showCursorToggled = false;
        }
        else
        {
            glm::ivec2 dims = game->Video().displayDims;
            prevMPos = glm::vec2(dims.x / 2.f, dims.y * 3 / 4.f);
            if (showCursorToggled) { curMPos = prevMPos; showCursorToggled = false; }
            else
            {
                curMPos = glm::vec2(xpos, ypos);
            }
            glfwSetCursorPos(game->window, prevMPos.x, prevMPos.y);
        }

        /*
        if (showCursor)
        {
            prevMPos = curMPos;
            if (game->mainWindowFocused)
            {
                double xpos, ypos; glfwGetCursorPos(game->window, &xpos, &ypos); curMPos = glm::vec2(xpos, ypos);
                if (game->fullscreen)
                {
                    if (game->t_adjustToWindowDims) curMPos /= game->t_windowDimsDivider;
                    else
                    {
                        glm::ivec2 winSize; glfwGetWindowSize(game->window, &winSize.x, &winSize.y);
                        curMPos -= glm::vec2(winSize - game->GetDisplayDims()) / 2.f;
                    }
                }
            }
            showCursorToggled = false;
        }
        else
        {
            glm::ivec2 dims = game->GetDisplayDims();
            prevMPos = glm::vec2(dims.x / 2.f, dims.y * 3 / 4.f);
            if (showCursorToggled) { curMPos = prevMPos; showCursorToggled = false; }
            else
            {
                double xpos, ypos; glfwGetCursorPos(game->window, &xpos, &ypos);
                curMPos = glm::vec2(xpos, ypos);
            }
            glfwSetCursorPos(game->window, prevMPos.x, prevMPos.y);
        }*/
    }

    void GameInput::SetShowCursor(bool show)
    {
        if (show != showCursor)
        {
            if (!show)
            {
                glm::ivec2 dims = game->Video().displayDims;
                glfwSetCursorPos(game->window, dims.x / 2.f, dims.y * 3 / 4.f);
            }
            glfwSetInputMode(game->window, GLFW_CURSOR, show ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_DISABLED);
            showCursorToggled = true;
        }
        showCursor = show;
    }

}