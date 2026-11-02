#include "Toolkit.hpp"
using namespace SGL;

namespace TLB
{
    // --- Camera3D ---
    Camera3D::Camera3D(float FOV, float farPlane, float nearPlane, glm::vec3 CameraUp) : FOV(FOV), yaw(0), pitch(0), pos(0, 0, 0),
        cameraUp(CameraUp), sensvty(0.4f), farPlane(farPlane), nearPlane(nearPlane)
    {
    }

    glm::mat4 Camera3D::GetViewMat() const
    {
        return glm::lookAt(pos, pos + GetCameraFront(), cameraUp);
    }

    glm::mat4 Camera3D::GetProjectionMat(int width, int height) const
    {
        if (height <= 0) { return glm::mat4(1.f); };
        return glm::perspective(glm::radians(FOV), (float)width / (float)height, nearPlane, farPlane);
    }

    glm::vec3 Camera3D::GetCameraFront() const
    {
        float radYaw = glm::radians(yaw), radPitch = glm::radians(pitch);
        float csYaw = cos(radYaw), snYaw = sin(radYaw);
        float csPitch = cos(radPitch), snPitch = sin(radPitch);
        return glm::vec3(snYaw * csPitch, snPitch, -csYaw * csPitch);
    }

    glm::vec3 Camera3D::GetCameraRight() const
    {
        return glm::cross(GetCameraFront(), cameraUp);
    }

    glm::vec3 Camera3D::GetCameraXZ() const
    {
        float radYaw = glm::radians(yaw);
        float csYaw = cos(radYaw), snYaw = sin(radYaw);
        return glm::vec3(snYaw, 0, -csYaw);
    }

    glm::vec3 Camera3D::GetCameraXZRight() const
    {
        return glm::cross(GetCameraXZ(), cameraUp);
    }

    void Camera3D::UpdateRotation(GLFWwindow* window, glm::vec2 prev, glm::vec2 cur)
    {
        (void)window;
        glm::vec2 change = cur - prev;
        change *= sensvty * 180 / 800.f;
        yaw += change.x;
        if (yaw > 180) yaw -= 360;
        else if (yaw <= -180) yaw += 360;
        pitch = std::clamp(pitch - change.y, -89.9f, 89.9f);
    }

    // --- RNG ---
    RNG::RNG(int seed) : gen(seed) {}
    RNG::RNG() : gen(std::random_device{}()) {}

    int RNG::Next(int min, int max)
    {
        dist.param(std::uniform_int_distribution<int>::param_type(min, max));
        return dist(gen);
    }

    int RNG::Next(int max)
    {
        return Next(0, max);
    }

    float RNG::NextFloat()
    {
        return dist_float(gen);
    }
}