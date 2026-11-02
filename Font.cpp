#include "Font.hpp"

namespace TLB
{
    Font::Font(int atlasPow) : _texture(GLObjectType::Texture), _loaded(false), _capacity(0), _charheight(0), _side((int)std::pow(2, atlasPow))
    {
        glBindTexture(GL_TEXTURE_2D, _texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _side, _side, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Font::GlBindBitmapTex() { glBindTexture(GL_TEXTURE_2D, _texture); }

    void Font::GlActivateBindBitmapTex(unsigned int location)
    {
        glActiveTexture(GL_TEXTURE0 + location);
        GlBindBitmapTex();
    }

    bool Font::Load(const char* path, uint32_t height)
    {
        _capacity = _side / height;
        _charheight = height;

        FT_Library ft;
        if (FT_Init_FreeType(&ft))
        {
            std::cout << "[Font] ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return false;
        }

        FT_Face face;
        if (FT_New_Face(ft, path, 0, &face))
        {
            std::cout << "[Font] ERROR::FREETYPE: Failed to load font " << std::endl;
            return false;
        }

        FT_Set_Pixel_Sizes(face, 0, height);

        glBindTexture(GL_TEXTURE_2D, _texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

        int widthCumul = 0;
        int currentRow = 0;
        auto GetWidthCumul = [&](int widthToAdd) -> int
            {
                if (widthCumul + widthToAdd >= _side)
                {
                    widthCumul = 0; currentRow++;
                }
                int oldWidth = widthCumul;
                widthCumul += widthToAdd + 1;
				return oldWidth;
            };

        for (uint c = 0; c < 2048; c++)
        {
            // load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "[Font] ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }

            int width = static_cast<int>(face->glyph->bitmap.width);
            int widthCumul = GetWidthCumul(width);
            // +1 is for vertical padding
            glTexSubImage2D(GL_TEXTURE_2D, 0, widthCumul, currentRow * (height + 1), width, face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

            // store character for later use
            Char character = {
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                face->glyph->advance.x, widthCumul, currentRow

            };
            chars.insert(std::pair<uint, Char>(c, character));
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        return true;
    }

    vector<TextVert> Font::RenderText(std::string text, glm::vec2 pos, float scale, glm::vec2 textOrigin,
        glm::vec4 tlCol, glm::vec4 trCol, glm::vec4 blCol, glm::vec4 brCol,
        glm::vec2* textRect)
    {
        vector<TextVert> textVerts;

        // Precompute frequently used constants
        const float trueScale = scale / _charheight;
        const float texelSize = 1.f / _side;
        const float charHeightInTexels = static_cast<float>(_charheight + 1) * texelSize;

        // Lambda to generate vertices for a single character quad
        auto GenerateQuad = [&](const Char& ch, float xpos, float ypos, float width, float height,
            const glm::vec4& topLeftCol, const glm::vec4& topRightCol,
            const glm::vec4& bottomLeftCol, const glm::vec4& bottomRightCol) -> vector<TextVert>
            {
                // Calculate texture coordinates for this character in the atlas
                const float texU = ch.widthCumul * texelSize;
                const float texV = ch.row * charHeightInTexels;
                const float texU2 = texU + ch.Size.x * texelSize;
                const float texV2 = texV + ch.Size.y * texelSize;

                // Generate two triangles forming a quad
                return vector<TextVert>{
                    // Triangle 1: top-left, bottom-left, bottom-right
                    { { xpos, ypos - height }, { texU,  texV }, topLeftCol },
                    { { xpos,         ypos }, { texU,  texV2 }, bottomLeftCol },
                    { { xpos + width, ypos }, { texU2, texV2 }, bottomRightCol },

                        // Triangle 2: top-left, bottom-right, top-right
                    { { xpos,         ypos - height }, { texU,  texV  }, topLeftCol },
                    { { xpos + width, ypos          }, { texU2, texV2 }, bottomRightCol },
                    { { xpos + width, ypos - height }, { texU2, texV  }, topRightCol }
                };
            };

        // Main character iteration loop
        float accumulatedAdvance = 0.f;

        for (auto charIter = text.begin(); charIter != text.end(); ++charIter)
        {
            const Char& ch = chars[*charIter];

            // Calculate position for this character
            const float xpos = pos.x + ch.Bearing.x * trueScale + accumulatedAdvance;
            const float ypos = pos.y + (ch.Size.y - ch.Bearing.y) * trueScale;
            const float width = ch.Size.x * trueScale;
            const float height = ch.Size.y * trueScale;

            // Generate main character quad
            vector<TextVert> charQuad = GenerateQuad(ch, xpos, ypos, width, height,
                tlCol, trCol, blCol, brCol);

            // Add main quad to output
            textVerts.insert(textVerts.end(),
                std::make_move_iterator(charQuad.begin()),
                std::make_move_iterator(charQuad.end()));

            // Advance cursor (Advance is in 1/64 pixels, shift by 6 to convert to pixels)
            accumulatedAdvance += (ch.Advance >> 6) * trueScale;
        }

        // Apply text origin offset (centering/alignment)
        const glm::vec2 textRectSize = glm::vec2(accumulatedAdvance, _charheight * trueScale);
        const glm::vec2 originOffset = glm::vec2(textOrigin.x, textOrigin.y - 1.f) * textRectSize;

        for (auto& vert : textVerts)
        {
            vert.pos -= originOffset;
        }

        // Output text rectangle if requested (positive height)
        if (textRect != nullptr)
        {
            *textRect = textRectSize;
        }

        return textVerts;
    }
}