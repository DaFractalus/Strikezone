#include "GLFWCallbacks.hpp"
#include "GameEngine.hpp"

namespace Engine
{
    void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
        GameEngine* game = static_cast<GameEngine*>(glfwGetWindowUserPointer(window));
        if (game && !game->Video().IGNORE_WIN_SIZE_CHANGES) game->UserEditedWindowSize(width, height);
    }

    void WindowFocusCallback(GLFWwindow* window, int focused)
    {
        GameEngine* game = static_cast<GameEngine*>(glfwGetWindowUserPointer(window));
        if (game) game->UserFocusedWindow(focused == GLFW_TRUE);
    }

    void WindowPosCallback(GLFWwindow* window, int xpos, int ypos)
    {
        GameEngine* game = static_cast<GameEngine*>(glfwGetWindowUserPointer(window));
        if (game) game->UserEditedWindowPos(xpos, ypos);
    }

    void GLFWErrorCallback(int error, const char* description)
    {
        std::cout << "[GLFW] Error %d: %s\n" << error << description;
    }

    void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        GameEngine* game = static_cast<GameEngine*>(glfwGetWindowUserPointer(window));
        if (game) game->KeyCallNotif(key, scancode, action, mods);
    }

    void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        GameEngine* game = static_cast<GameEngine*>(glfwGetWindowUserPointer(window));
        if (game) game->MouseButtonCallNotif(button, action, mods);
    }

    void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
    {
        GameEngine* game = static_cast<GameEngine*>(glfwGetWindowUserPointer(window));
        if (game) game->ScrollCallNotif(xOffset, yOffset);
    }

    void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        GameEngine* game = static_cast<GameEngine*>(glfwGetWindowUserPointer(window));
        if (game) game->CursorPosCallNotif(xpos, ypos);
    }
}