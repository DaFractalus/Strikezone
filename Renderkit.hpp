#pragma once

#include "SafeGL.hpp"

using namespace SGL;
namespace TLB
{
	// - Shader -
	class Texture;
	class Atlas;
	class Font;
	class Shader
	{
		std::unordered_map<std::string, GLint> _uniformMap;
		unsigned int ID;
	public:
		bool loaded;

		Shader();
		~Shader();
		void Load(const char* vertexPath, const char* fragmentPath);
		bool GlUseShader();
		GLint GetUniformLocation(const std::string& name, bool ignr = false);
		bool SetSamplerUniform(const char* name, Texture& texture, uint32_t slot, bool ignore = false);
		bool SetSamplerUniform(const char* name, Atlas& atlas, uint32_t slot, bool ignore = false);
		bool SetSamplerUniform(const char* name, Font& font, uint32_t slot, bool ignore = false);
		bool SetUniform(const char* n, const int v, bool ignr = false);
		bool SetUniform(const char* n, const float v, bool ignr = false);
		bool SetUniform(const char* n, const glm::vec2 v, bool ignr = false);
		bool SetUniform(const char* n, const glm::vec3 v, bool ignr = false);
		bool SetUniform(const char* n, const glm::vec4 v, bool ignr = false);
		bool SetUniform(const char* n, const glm::mat3 v, bool ignr = false);
		bool SetUniform(const char* n, const glm::mat4 v, bool ignr = false);

	};
	//- RenderBuffer -
	struct RBFormatParam
	{
		GLenum internalformat, format, datatype;
	};

	class RenderBuffer
	{
	public:
		SmartGLID ID, _dpthTextureId;
		vector<SmartGLID> _colrTextureIds;
		int width, height;
		bool ready, hasDepth;
		vector<RBFormatParam> bufferParams;

		RenderBuffer();
		void Create(int width, int height, bool useDepth, vector<RBFormatParam> formatParams);
		bool GLBindBuffer();
		bool StartRender(int colorAttachement = 0);
		bool GlBindTexture(unsigned int location, int colorAttachmentIndex = 0);
		bool GlBindDepthTexture(unsigned int location);
		void Resize(int newWidth, int newHeight);

	};

	//
	//Vertex
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoords;
	};
}