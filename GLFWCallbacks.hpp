#pragma once

namespace Engine
{
    void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    void WindowFocusCallback(GLFWwindow* window, int focused);
    void WindowPosCallback(GLFWwindow* window, int xpos, int ypos);
    void GLFWErrorCallback(int error, const char* description);
    void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
    void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);

    inline bool WasKeyPressed(int detect_key, int event_key, int action)
    {
        return action == GLFW_PRESS && detect_key == event_key;
    }

    inline bool WasKeyReleased(int detect_key, int event_key, int action)
    {
        return action == GLFW_RELEASE && detect_key == event_key;
    }
}