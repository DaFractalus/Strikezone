#include "Textures.hpp"

namespace TLB
{
    //--- Texture ---
    Texture::Texture() : ID(GLObjectType::Texture), width(0), height(0), nrChannels(-1), loaded(false) {}
    int Texture::getWidth() const { return width; }
    int Texture::getHeight() const { return height; }

    void Texture::Load(const std::string& png_path, bool inSRGB, bool flipped)
    {
        loaded = ID.ready();
        stbi_set_flip_vertically_on_load(flipped);

        unsigned char* data = stbi_load(png_path.c_str(), &width, &height, &nrChannels, 0);
        if (!data)
        {
            std::cout << "[Texture] unable to find texture [" << png_path << "] - using default.\n";
            stbi_image_free(data);
            data = stbi_load("assets/default.png", &width, &height, &nrChannels, 0);
            if (!data) { std::cout << "[Texture] what the hell..."; stbi_image_free(data); return; }
        }

        //Generating Opengl Texture
        GLenum format = nrChannels == 1 ? GL_RED : (nrChannels == 3 ? GL_RGB : GL_RGBA);
        GLenum internalformat = inSRGB ? (nrChannels == 1 ? GL_RED : (nrChannels == 3 ? GL_SRGB : GL_SRGB_ALPHA)) : format;

        glBindTexture(GL_TEXTURE_2D, ID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_image_free(data);
    }

    bool Texture::GlBindTexture() const
    {
        glBindTexture(GL_TEXTURE_2D, ID);
        return loaded;
    }

    bool Texture::GlActivateBindTexture(unsigned int location) const
    {
        glActiveTexture(GL_TEXTURE0 + location);
        return GlBindTexture();
    }

    //--- CubeMap ---
    CubeMap::CubeMap() : ID(GLObjectType::Texture), loaded(false) {}

    void CubeMap::Load(vector<std::string> faces, bool inSRGB)
    {
        loaded = ID.ready();
        glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0, inSRGB ? GL_SRGB : GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
                );
                stbi_image_free(data);
            }
            else
            {
                std::cout << "[CubeMap] Cubemap tex failed to load at path: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    }

    bool CubeMap::GlBindCubeMap()
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
        return loaded;
    }

    bool CubeMap::GlActivateBindCubeMap(unsigned int location)
    {
        glActiveTexture(GL_TEXTURE0 + location);
        return GlBindCubeMap();
    }
}