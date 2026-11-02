#ifndef CONTROLS_HPP
#define CONTROLS_HPP

namespace Engine
{
    class GameEngine;
}

namespace TLB
{
    enum class KeyBind
    {
        Forward, Backward, Left, Right, Jump, Sneak, Sprint, Pause,
        ToggleFullscreen, OpenInventory, ReturnBack, ToggleDebug, LowerFOV, RaiseFOV
    };

    class GameInput
    {
        std::unordered_map<KeyBind, int> kbInputTable;
        bool prevMBL, prevMBM, prevMBR, curMBL, curMBM, curMBR;

    public:
        Engine::GameEngine* game;
        double mouseScroll;
        glm::vec2 curMPos;
        glm::vec2 prevMPos;

        bool KeyboardCaptured, MouseCaptured;
        bool showCursor, showCursorToggled;

        GameInput(Engine::GameEngine& game);
        bool CurMouseButton(int button, bool forced = false) const;
        bool PrevMouseButton(int button, bool forced = false) const;
        bool IsCurrentKeyPressed(KeyBind keyBind, bool forced = false) const;
        bool WasKeyClicked(KeyBind keyBind, int event_key, int action, bool force = false) const;
        bool WasMButtonClicked(int button, int event_button, int action, bool force = false) const;
        void UpdateInputStates();
        void UpdateCursorPos(double xpos, double ypos);
        void SetShowCursor(bool show);
        //inline void MakeDoneKeyboard(bool state = true) { doneKeyboard |= state; }
        inline void MakeDoneMouse(bool state = true) { MouseCaptured |= state; }
    };
}

#endif