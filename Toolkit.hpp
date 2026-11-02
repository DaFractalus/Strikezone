#pragma once

#include "SafeGL.hpp"
#include "Renderkit.hpp"
#include "Textures.hpp"

namespace TLB
{
	inline constexpr double PI = 3.14159265358979323846;
	inline constexpr double TAU = PI * 2;

	// - Camera3D -
	class Camera3D
	{
	public:
		float FOV, yaw, pitch;
		glm::vec3 pos;
		glm::vec3 cameraUp;
		float sensvty;
		float farPlane, nearPlane;

		Camera3D(float FOV, float farPlane = 1000.f, float nearPlane = 0.1f, glm::vec3 CameraUp = glm::vec3(0, 1, 0));
		glm::mat4 GetViewMat() const;
		glm::mat4 GetProjectionMat(int width, int height) const;
		glm::vec3 GetCameraRight() const;
		glm::vec3 GetCameraFront() const;
		glm::vec3 GetCameraXZ() const;
		glm::vec3 GetCameraXZRight() const;
		void UpdateRotation(GLFWwindow* window, glm::vec2 prev, glm::vec2 cur);

	};

	// - RNG -
	struct RNG
	{
		std::mt19937 gen;
		std::uniform_int_distribution<int> dist;
		std::uniform_real_distribution<float> dist_float;
		RNG(int seed);
		RNG();
		int Next(int min, int max);
		int Next(int max);
		float NextFloat();
	};
}