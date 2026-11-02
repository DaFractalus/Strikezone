#include "Atlas.hpp"

namespace TLB
{
    //SpriteRect---------------------
    SpriteRect::SpriteRect(int x, int y, int w, int h, int ext_w, int ext_h) :
        X(x), Y(y), W(w), H(h), EXT_W(ext_w), EXT_H(ext_h) {
    }

    int SpriteRect::getOriginW(int unit) const { return W * unit - EXT_W; }
    int SpriteRect::getOriginH(int unit) const { return H * unit - EXT_H; }

    //Atlas ----------------------
    Atlas::Atlas(int unitsize, int width_capacity, bool is_srgb) : _unit(unitsize), _capacity(width_capacity),
        _levels(static_cast<int>(std::log2(unitsize)) + 1), _texture(GLObjectType::Texture)
    {
        int texSide = width_capacity * unitsize;
        glBindTexture(GL_TEXTURE_2D, _texture);
        for (int level = 0; level < _levels; ++level)
        {
            int w = texSide >> level;
            glTexImage2D(GL_TEXTURE_2D, level, is_srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8, w, w, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, _levels - 1);

        glBindTexture(GL_TEXTURE_2D, 0);
        _cellState.resize(width_capacity * width_capacity, false);


        // Creating Default Textures ----------------
        //NoTXT
        vector<uint8_t> notxt;
        notxt.resize(16 * 16 * 4);

        for (uint32_t y = 0; y < 16; y++)
        {
            for (uint32_t x = 0; x < 16; x++)
            {
                bool pink = (x < 8) == (y < 8);
                notxt[4 * (y * 16 + x)] = pink ? 255 : 0;
                notxt[4 * (y * 16 + x) + 1] = pink ? 0 : 32;
                notxt[4 * (y * 16 + x) + 2] = pink ? 196 : 96;
                notxt[4 * (y * 16 + x) + 3] = 255;
            }
        }
        PackImageData(-1, 16, 16, 4, notxt.data());

        //White
        for (uint32_t i = 0; i < 16 * 16 * 4; i++) { notxt[i] = 255; }
        PackImageData(-2, 16, 16, 4, notxt.data());
    }

    int Atlas::GetUnit() const { return _unit; }

    int Atlas::GetCapacity() const { return _capacity; }

    void Atlas::GlBindAtlasTex() { glBindTexture(GL_TEXTURE_2D, _texture); }

    void Atlas::GlActivateBindAtlasTex(unsigned int location)
    {
        glActiveTexture(GL_TEXTURE0 + location);
        GlBindAtlasTex();
    }

    const SpriteRect& Atlas::GetSprite(int id) const
    {
        auto it = _sprites.find(id);
        if (it == _sprites.end()) { /*std::cout << " [Atlas] | WARNING: no existing texture id [" << id << "]\n";*/ return *_sprites.at(-1); }
        return *it->second;
    }

    bool Atlas::PackTexture(int id, std::string path)
    {
        stbi_set_flip_vertically_on_load(true);
        int width, height, channels;
        uint8_t* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (!data)
        {
            std::cout << "[Atlas] | WARNING: unable to find texture [" << path << "].\n";
            stbi_image_free(data);
            return false;
            //data = stbi_load("assets/default.png", &width, &height, &channels, 0);
            //stbi_set_flip_vertically_on_load(false);
            //if (!data) { std::cout << "what the hell..."; stbi_image_free(data); return false;}
        }
        stbi_set_flip_vertically_on_load(false);

        bool success = PackImageData(id, width, height, channels, data);
        stbi_image_free(data);
        return success;

    }

    bool Atlas::PackImageData(int id, int width, int height, int channels, uint8_t* data)
    {
        //if the id already exists, reset back the cells occupied by the previous sprite id holder
        auto pIt = _sprites.find(id);
        if (pIt != _sprites.end())
        {
            //TODO: THIS SHOULD ALSO REMOVE PIXELS NOT COVERED BY NEW TRANSPARENT PIXELS (use overriding blendmode!)
            auto& pSprt = pIt->second;
            for (int sy = 0; sy < pSprt->H; sy++) {
                for (int sx = 0; sx < pSprt->W; sx++)
                {
                    _cellState[(sy + pSprt->Y) * _capacity + (sx + pSprt->X)] = false;
                }
            }
        }

        /***********/
        int W = ceilf(width / (float)_unit), H = ceilf(height / (float)_unit), X, Y;
        int extendW = W * _unit - width, extendH = H * _unit - height;

        bool packed = false;
        for (size_t cY = 0; cY < _capacity && !packed; cY++) //iterate by row if no packing space found yet
        {
            if (cY + H > _capacity) { break; } //if texture gets out of bounds, stop

            for (size_t cX = 0; cX < _capacity; cX++)
            {
                if (cX + W > _capacity) { break; } //if texture gets out of bounds, stop and go to next row

                bool overlap = false;
                for (int sy = 0; sy < H && !overlap; sy++) {
                    for (int sx = W - 1; sx >= 0; sx--) //iterate y-x, backwards x to overlap close to edge
                    {
                        if (_cellState[(sy + cY) * _capacity + (sx + cX)]) { cX += sx; overlap = true; break; }
                    }
                }
                //if a cell is occupied, jump over the overlap edge in x

                //else if no overlap found, record current dimensions and indicate a packing space found
                if (!overlap)
                {
                    X = cX; Y = cY; packed = true; break;
                }

            }
        }

        if (packed) //packing space found, make a SpriteRect with id hash, and draw into the atlas
        {
            _sprites[id] = std::make_unique<SpriteRect>(X, Y, W, H, extendW, extendH);
            for (int sy = 0; sy < H; sy++)
            {
                for (int sx = 0; sx < W; sx++)
                {
                    _cellState[(sy + Y) * _capacity + (sx + X)] = true;
                }
            }

            std::vector<std::vector<uint8_t>> mipmaps;
            int nextWidth = width;
            int nextHeight = height;
            uint8_t* currentPixels = data;

            mipmaps.push_back(std::vector<uint8_t>(currentPixels, currentPixels + width * height * channels));

            for (int l = 0; l < _levels - 1; l++)
            {
                nextWidth = std::max(1, nextWidth / 2);
                nextHeight = std::max(1, nextHeight / 2);

                std::vector<uint8_t> nextLevel(nextWidth * nextHeight * channels);

                stbir_resize_uint8_linear(data, width, height, 0,
                    nextLevel.data(), nextWidth, nextHeight, 0, static_cast<stbir_pixel_layout>(channels));

                mipmaps.push_back(nextLevel);
            }

            auto format = channels == 4 ? GL_RGBA : (channels == 3 ? GL_RGB : (channels == 2 ? GL_RG : GL_RED));
            int scale = 1;
            glBindTexture(GL_TEXTURE_2D, _texture);
            for (int lvl = 0; lvl < _levels; lvl++)
            {
                glTexSubImage2D(GL_TEXTURE_2D, lvl, X * _unit / scale, Y * _unit / scale, width / scale, height / scale, format, GL_UNSIGNED_BYTE, mipmaps[lvl].data());
                scale *= 2;
            }

            glBindTexture(GL_TEXTURE_2D, 0);
            return true;

        }
        return false;
    }
}