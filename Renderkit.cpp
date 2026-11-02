#include "Renderkit.hpp"
#include "Textures.hpp"
#include "Atlas.hpp"
#include "Font.hpp"

namespace TLB
{
    // --- Shader --- 
    Shader::Shader() : ID(0), loaded(false) {}

    void Shader::Load(const char* vertexPath, const char* fragmentPath)
    {
        _uniformMap.clear();
        std::string vertCode, fragCode;
        std::ifstream vShaderFile, fShaderFile;

        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;

            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            vShaderFile.close();
            fShaderFile.close();

            vertCode = vShaderStream.str();
            fragCode = fShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            (void)e;
            std::cout << "[Shader] ERROR: SHADER FILE NOT SUCCESFULLY READ D:`";
            return;
        }

        const char* vertShaderCode = vertCode.c_str();
        const char* fragShaderCode = fragCode.c_str();

        unsigned int vertexShader = 0;
        Safe_GlCreateShader(GL_VERTEX_SHADER, &vertexShader);
        glShaderSource(vertexShader, 1, &vertShaderCode, NULL);
        glCompileShader(vertexShader);

        int success_v; char infoLog_v[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success_v);
        if (!success_v)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog_v);
            { std::cout << "[Shader] ERROR VERTEX SHADER COMPILATION FAILED ~A~\n" << infoLog_v << std::endl; return; }
        }

        unsigned int fragmentShader = 0;
        Safe_GlCreateShader(GL_FRAGMENT_SHADER, &fragmentShader);
        glShaderSource(fragmentShader, 1, &fragShaderCode, NULL);
        glCompileShader(fragmentShader);
        int success_f; char infoLog_f[512];
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success_f);
        if (!success_f)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog_f);
            { std::cout << "[Shader] ERROR FRAGMENT SHADER COMPILATION FAILED oAo\n" << infoLog_f << std::endl; return; }
        }

        Safe_GlCreateProgram(&ID);
        glAttachShader(ID, vertexShader);
        glAttachShader(ID, fragmentShader);
        glLinkProgram(ID);
        int success_p; char infoLog_p[512];
        glGetProgramiv(ID, GL_LINK_STATUS, &success_p);
        if (!success_p)
        {
            glGetProgramInfoLog(ID, 512, NULL, infoLog_p);
            {
                std::cout << "[Shader] ERROR SHADER PROGRAM LINKING FAILED @~@\n" << infoLog_p << std::endl;
                Safe_GlDeleteProgram(ID);
                return;
            }
        }

        Safe_GlDeleteShader(vertexShader);
        Safe_GlDeleteShader(fragmentShader);
        loaded = true;
    }

    bool Shader::GlUseShader()
    {
        glUseProgram(ID); return loaded;
    }

    GLint Shader::GetUniformLocation(const std::string& name, bool ignr)
    {
        auto it = _uniformMap.find(name);
        if (it != _uniformMap.end()) return it->second;

        auto location = glGetUniformLocation(ID, name.c_str());
        if (location < 0 && !ignr) { std::cout << "[Shader] WARNING: - \"" << name << "\" Uniform Location Wasn't Found.\n"; }

        _uniformMap.emplace(name, location);
        return location;
    }

    bool Shader::SetUniform(const char* n, const int v, bool ignr)
    {
        auto loc = GetUniformLocation(n, ignr); if (loc < 0) { return false; }
        glUniform1i(loc, v);  return true;
    }

    bool Shader::SetUniform(const char* n, const float v, bool ignr)
    {
        auto loc = GetUniformLocation(n, ignr); if (loc < 0) { return false; }
        glUniform1f(loc, v);  return true;
    }

    bool Shader::SetUniform(const char* n, const glm::vec2 v, bool ignr)
    {
        auto loc = GetUniformLocation(n, ignr); if (loc < 0) { return false; }
        glUniform2f(loc, v.x, v.y);  return true;
    }

    bool Shader::SetUniform(const char* n, const glm::vec3 v, bool ignr)
    {
        auto loc = GetUniformLocation(n, ignr); if (loc < 0) { return false; }
        glUniform3f(loc, v.x, v.y, v.z);  return true;
    }

    bool Shader::SetUniform(const char* n, const glm::vec4 v, bool ignr)
    {
        auto loc = GetUniformLocation(n, ignr); if (loc < 0) { return false; }
        glUniform4f(loc, v.x, v.y, v.z, v.w);  return true;
    }

    bool Shader::SetUniform(const char* n, const glm::mat3 v, bool ignr)
    {
        auto loc = GetUniformLocation(n, ignr); if (loc < 0) { return false; }
        glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(v));  return true;
    }

    bool Shader::SetUniform(const char* n, const glm::mat4 v, bool ignr)
    {
        auto loc = GetUniformLocation(n, ignr); if (loc < 0) { return false; }
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(v));  return true;
    }

    bool Shader::SetSamplerUniform(const char* name, Texture& texture, uint32_t slot, bool ignore)
    {
        texture.GlActivateBindTexture(slot);
        auto loc = GetUniformLocation(name, ignore); if (loc < 0) { return false; }
        glUniform1i(loc, slot);  return true;
    }

    bool Shader::SetSamplerUniform(const char* name, Atlas& atlas, uint32_t slot, bool ignore)
    {
        atlas.GlActivateBindAtlasTex(slot);
        auto loc = GetUniformLocation(name, ignore); if (loc < 0) { return false; }
        glUniform1i(loc, slot);  return true;
    }

    bool Shader::SetSamplerUniform(const char* name, Font& font, uint32_t slot, bool ignore)
    {
        font.GlActivateBindBitmapTex(slot);
        auto loc = GetUniformLocation(name, ignore); if (loc < 0) { return false; }
        glUniform1i(loc, slot);  return true;
    }

    Shader::~Shader()
    {
        if (loaded) Safe_GlDeleteProgram(ID);
    }

    //--- RenderBuffer ---
    RenderBuffer::RenderBuffer() : ID(GLObjectType::Framebuffer), _dpthTextureId(GLObjectType::Texture),
        width(0), height(0), ready(false), hasDepth(false)
    {
    }

    void RenderBuffer::Create(int width, int height, bool useDepth, vector<RBFormatParam> formatParams)
    {
        this->width = width; this->height = height;
        bufferParams = formatParams; hasDepth = useDepth;
        int colorAttachmentCount = bufferParams.size();
        glBindFramebuffer(GL_FRAMEBUFFER, ID);

        _colrTextureIds.reserve(colorAttachmentCount);
        for (int a = 0; a < colorAttachmentCount; a++)
        {
            _colrTextureIds.emplace_back(GLObjectType::Texture);
        }

        GLenum target = GL_TEXTURE_2D;
        std::vector<GLenum> drawBuffers;

        for (int i = 0; i < colorAttachmentCount; ++i)
        {
            glBindTexture(target, _colrTextureIds[i]);

            glTexImage2D(target, 0, formatParams[i].internalformat, width, height, 0, formatParams[i].format, formatParams[i].datatype, nullptr);
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, target, _colrTextureIds[i], 0);
            drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
        }

        if (colorAttachmentCount > 0)
        {
            glDrawBuffers(drawBuffers.size(), drawBuffers.data());
        }
        else
        {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }

        // Create and attach depth texture
        if (useDepth)
        {
            glBindTexture(target, _dpthTextureId);


            glTexImage2D(target, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, _dpthTextureId, 0);
            hasDepth = true;
        }

        // Final check
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "[RenderBuffer] ERROR: FRAMEBUFFER NOT COMPLETE!\n";
        }
        else { ready = true; }

    }

    bool RenderBuffer::GLBindBuffer()
    {
        if (!ready) return false;
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        return true;
    }

    bool RenderBuffer::StartRender(int colorAttachement)
    {
        if (!GLBindBuffer()) return false;
        glViewport(0, 0, width, height);
        if (hasDepth) { glEnable(GL_DEPTH_TEST); }
        else { glDisable(GL_DEPTH_TEST); }
        glDrawBuffer(GL_COLOR_ATTACHMENT0 + colorAttachement);
        return true;
    }

    void RenderBuffer::Resize(int newWidth, int newHeight)
    {
        this->width = newWidth;
        this->height = newHeight;

        for (int a = 0; a < bufferParams.size(); a++)
        {
            glBindTexture(GL_TEXTURE_2D, _colrTextureIds[a]);
            glTexImage2D(GL_TEXTURE_2D, 0, bufferParams[a].internalformat, width, height, 0, bufferParams[a].format, bufferParams[a].datatype, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        if (hasDepth)
        {
            glBindTexture(GL_TEXTURE_2D, _dpthTextureId);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    bool RenderBuffer::GlBindTexture(unsigned int location, int colorAttachmentIndex)
    {
        if (colorAttachmentIndex >= bufferParams.size() || !ready) return false;
        glActiveTexture(GL_TEXTURE0 + location);
        glBindTexture(GL_TEXTURE_2D, _colrTextureIds[colorAttachmentIndex]);
        return true;
    }

    bool RenderBuffer::GlBindDepthTexture(unsigned int location)
    {
        if (!hasDepth || !ready) return false;
        glActiveTexture(GL_TEXTURE0 + location);
        glBindTexture(GL_TEXTURE_2D, _dpthTextureId);
        return true;
    }
}