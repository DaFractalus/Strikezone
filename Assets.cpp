#include "Assets.hpp"
#include "ModelImport.hpp"
#include "Audio.hpp"
#include "Text.hpp"
#include "UIWidget.hpp"

namespace TLB
{
    AssetManager::AssetManager() :
        uiatlas(16, 64, false), uifont(10),// atlas(16, 64, true),
        //itematlas(16, 64, false), audio(), lang(),
        skyVAO(GLObjectType::VertexArray), skyVBO(GLObjectType::Buffer),
        cursorVAO(GLObjectType::VertexArray), cursorVBO(GLObjectType::Buffer)
    {

    }

    void AssetManager::LoadAssets()
    {
        //Preventing other threads from accessing assets while loading.
        std::unique_lock lock(assetMutex);

        //Cursor
        {
            cursorVAO.setVAOLayout([this]()
                {
                    glBindVertexArray(cursorVAO);
                    glBindBuffer(GL_ARRAY_BUFFER, cursorVBO);

                    glEnableVertexAttribArray(0);
                    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(UIB::UIVert), (void*)0);
                    glEnableVertexAttribArray(1);
                    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(UIB::UIVert), (void*)offsetof(UIB::UIVert, uvs));
                    glEnableVertexAttribArray(2);
                    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(UIB::UIVert), (void*)offsetof(UIB::UIVert, color));

                    glBindVertexArray(0);
                });

            auto& rect = uiatlas.GetSprite(-2);
            float tileIndex = (float)(rect.X + uiatlas.GetCapacity() * rect.Y);

            glm::vec4 outlineCol(0, 0.1f, 0.2f, 1);
            glm::vec4 highlightCol(1, 0.8f, 0.f, 1);
            glm::vec4 aboveCol(0.9f, 0.95f, 1, 1);
            glm::vec4 belowCol = aboveCol * 0.75f;
            
            int size = 24;
            int hfsize = size * .5f;
            int outlineW = 2;
            vector<UIB::UIVert> verts = {
                //Outline
                {glm::vec3(0, 0, 0), tileIndex, glm::vec2(0), outlineCol},
                {glm::vec3(hfsize , hfsize , 0), tileIndex, glm::vec2(0), outlineCol},
                {glm::vec3(hfsize - outlineW, size, 0), tileIndex, glm::vec2(0), outlineCol},

                {glm::vec3(0, 0, 0), tileIndex, glm::vec2(0), outlineCol},
                {glm::vec3(hfsize , hfsize , 0), tileIndex, glm::vec2(0), outlineCol},
                {glm::vec3(size, hfsize - outlineW, 0), tileIndex, glm::vec2(0), outlineCol},
                //highlight
                {glm::vec3(hfsize *(1 + 0.25f), hfsize *(1 + 0.25f), 0), tileIndex, glm::vec2(0), highlightCol},
                {glm::vec3(size - hfsize * 0.25f, size - hfsize * 0.25f, 0), tileIndex, glm::vec2(0), highlightCol},
                {glm::vec3(hfsize *(1.5f + 0.2f), hfsize *(1.5f - 0.2f), 0), tileIndex, glm::vec2(0), highlightCol},
                {glm::vec3(hfsize * (1 + 0.25f), hfsize * (1 + 0.25f), 0), tileIndex, glm::vec2(0), highlightCol},
                {glm::vec3(size - hfsize * 0.25f, size - hfsize * 0.25f, 0), tileIndex, glm::vec2(0), highlightCol},
                {glm::vec3(hfsize * (1.5f - 0.2f), hfsize * (1.5f + 0.2f), 0), tileIndex, glm::vec2(0), highlightCol},
                //above
                {glm::vec3(outlineW, outlineW, 0), tileIndex, glm::vec2(0), aboveCol},
                {glm::vec3(hfsize - outlineW / 2.f, hfsize - outlineW / 2.f, 0), tileIndex, glm::vec2(0), aboveCol},
                {glm::vec3(size - outlineW, hfsize - outlineW, 0), tileIndex, glm::vec2(0), aboveCol},
                //below
                {glm::vec3(outlineW, outlineW, 0), tileIndex, glm::vec2(0), belowCol},
                {glm::vec3(hfsize - outlineW / 2.f, hfsize - outlineW / 2.f, 0), tileIndex, glm::vec2(0), belowCol},
                {glm::vec3(hfsize - outlineW, size - outlineW, 0), tileIndex, glm::vec2(0), belowCol},
            };

            cursorVBO.uploadBufferData(verts, GL_ARRAY_BUFFER, GL_STATIC_DRAW);
        }
        
        //Tiles
        //GenerateTileMeshes();

        //Fonts
        //auto fontPath = "assets/Atkinson-Hyperlegible-Bold-102.otf";
        //auto fontPath = "assets/Atkinson-Hyperlegible-Regular-102.otf";
        //auto fontPath = "assets/Boldpixels.ttf";
        //auto fontPath = "assets/commod/cmunbi.ttf";
        uifont.Load("assets/Bender-Bold.otf", 48);

        //UI
        /* ui stuff */

        //Shaders
        engineScreenVF.Load("assets/shaders/enginescreen.vert", "assets/shaders/screen.frag");
        uiShader.Load("assets/shaders/ui.vert", "assets/shaders/ui.frag");
        panelShader.Load("assets/shaders/simple.vert", "assets/shaders/panel.frag");
        textShader.Load("assets/shaders/text.vert", "assets/shaders/text.frag");
        uisepShader.Load("assets/shaders/uiseperator.vert", "assets/shaders/uiseperator.frag");
        /*
        blendShader.Load("assets/shaders/screen.vert", "assets/shaders/blend.frag");
        //-------------//
        //lightShader.Load("assets/shaders/basic.vert", "assets/shaders/lightcolor.frag");
        bareShader.Load("assets/shaders/basic.vert", "assets/shaders/basiccolor.frag");
        skyshader.Load("assets/shaders/sky.vert", "assets/shaders/sky.frag");
        //normalShader.Load("assets/basic.vert", "assets/normal.frag");
        lineShader.Load("assets/shaders/line.vert", "assets/shaders/line.frag");
        gBufferShader.Load("assets/shaders/basic.vert", "assets/shaders/gbuffer.frag");
        gBufAnimShader.Load("assets/shaders/Anbasic.vert", "assets/shaders/gbuffer.frag");
        ssaoShader.Load("assets/shaders/screenBlur.vert", "assets/shaders/ssao.frag");
        ssaoblurShader.Load("assets/shaders/screenBlur.vert", "assets/shaders/ssaoblur.frag");
        lindepthShader.Load("assets/shaders/screen.vert", "assets/shaders/lindepth.frag");
        bplightShader.Load("assets/shaders/screen.vert", "assets/shaders/bplight.frag");
        tergBufferShader.Load("assets/shaders/terbasic.vert", "assets/shaders/tergbuffer.frag");
        gBufferInstancedClrShader.Load("assets/shaders/basic_InstancedClr.vert", "assets/shaders/gbuffer_InstancedClr.frag");
        cloudNoiseShader.Load("assets/shaders/screen.vert", "assets/shaders/cloudNoise.frag");
        doublePassBlurShader.Load("assets/shaders/screenBlur.vert", "assets/shaders/doublePassBlur.frag");
        cloudShader.Load("assets/shaders/cloud.vert", "assets/shaders/cloud.frag");
        cloudBlurShader.Load("assets/shaders/screenBlur.vert", "assets/shaders/cloudBlur.frag");
        screenApplyShader.Load("assets/shaders/screen.vert", "assets/shaders/screenApply.frag");
        gridShader.Load("assets/shaders/grid.vert", "assets/shaders/grid.frag");
        */

        //sky
        vector<std::string> faces
        { "assets/sbs/sb1/sb_right.png",
          "assets/sbs/sb1/sb_left.png",
          "assets/sbs/sb1/sb_top.png",
          "assets/sbs/sb1/sb_bottom.png",
          "assets/sbs/sb1/sb_front.png",
          "assets/sbs/sb1/sb_back.png" };
        //skybox.Load(faces);

        static const std::vector<float> skyboxVertices = {
            // positions          
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f
        };

        glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
        skyVBO.uploadBufferData(skyboxVertices, GL_ARRAY_BUFFER, GL_STATIC_DRAW);

        skyVAO.setVAOLayout([this]()
            {
                glBindVertexArray(skyVAO);
                glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
            });
        glBindVertexArray(0);

        //Tiles //TODO: should be totally replaced with the json blockstates system and the textures are decided by the models
        vector<std::string> tpaths = { "assets/default.png", "assets/tiles/stone.png", "assets/tiles/clay.png", "assets/tiles/grass.png" };
        int id = 0;
        for (auto& tpath : tpaths)
        {
            //atlas.PackTexture(id, tpath);
            id++;
        }

        //Items
        /* item stuff */

        //Audio
        //audio.LoadSound(0, "assets/sounds/ui_scroll.ogg");
        //audio.LoadSound(1, "assets/sounds/stone_hit.ogg");
        //audio.LoadSound(2, "assets/sounds/ambient_Wind.ogg");

        //Language
        lang.LoadLanguage("assets/languages/en_us.json");
    }

    void AssetManager::GenerateTileMeshes()
    {
        int version = 2;
        if (version == 1)
        {
            //Base Tile Bytes
            static std::unordered_set<uint> premades{
                0x000, 0x001, 0x005, 0x007, 0x011, 0x015, 0x017, 0x01D, 0x01F, 0x055, 0x057, 0x05F, 0x077, 0x07F, 0x0FF,
                0x100, 0x101, 0x105, 0x107, 0x111, 0x115, 0x117, 0x11D, 0x11F, 0x155, 0x157, 0x15F, 0x177, 0x17F, 0x1FF };
            static const char hexChars[] = "0123456789abcdef";
            ModelImport loadModel;

            std::string buffer;
            buffer.reserve(64);

            //Importing base Tiles
            for (uint b : premades)
            {
                buffer.clear();
                buffer += "assets/3dtiles/";

                if (b > 0xFF) buffer += hexChars[b >> 8];
                if (b > 0x0F) buffer += hexChars[(b >> 4) & 0x0F];
                buffer += hexChars[b & 0x0F];
                buffer += ".obj";

                loadModel.ImportPath(buffer, false);
                if (!loadModel.fullImport) continue;
                auto& mesh = loadModel.meshes[0];
                const auto& impVerts = mesh.verts;
                const auto& impInds = mesh.inds;

                //Seperaing triangles
                vector<Vertex> verts; verts.reserve(impInds.size());
                vector<uint> inds; inds.reserve(impInds.size());

                auto triangleCount = impInds.size() / 3;
                for (uint t = 0; t < triangleCount; t++)
                {
                    inds.emplace_back(t * 3); inds.emplace_back(t * 3 + 1); inds.emplace_back(t * 3 + 2);
                    int vi[3] = { t * 3, t * 3 + 1, t * 3 + 2 };
                    for (int v = 0; v < 3; v++)
                    {
                        uint impInd = impInds[vi[v]];
                        verts.emplace_back(mesh.verts[impInd], mesh.norms[impInd], mesh.uvs[impInd]);
                    }

                    glm::vec3 normal = glm::normalize(glm::cross(verts[vi[1]].position - verts[vi[0]].position, verts[vi[2]].position - verts[vi[0]].position));
                    verts[vi[0]].normal = normal; verts[vi[1]].normal = normal; verts[vi[2]].normal = normal;
                }

                tileMeshes.emplace(b, std::move(MemMesh(verts, inds)));

            }

            //Creating the other Tile rotations and vertical symetries
            static auto rotateBits = [](uint b) -> uint
                {
                    uint shell = ((b << 2) | ((b >> 6) & 3u)) & 0xFFu;
                    return shell | (b & (3u << 8));
                };

            static auto flipCovers = [](uint b) -> uint
                {
                    uint x = ((b >> 8) ^ (b >> 9)) & 1u;
                    return b ^ (x << 8) ^ (x << 9);
                };

            for (uint B = 1; B < 0x300; B++)
            {
                bool assignedMesh = false;
                for (int s = 0; !assignedMesh && s < 2; s++) //if no rotations exists, look for vertical symetries
                {
                    bool takeSymetry = s;
                    if (tileMeshes.contains(B)) { assignedMesh = true; break; }

                    uint rB = takeSymetry ? flipCovers(B) : B;
                    for (int r = 0; r < 4; r++)
                    {
                        auto it = tileMeshes.find(rB);
                        rB = rotateBits(rB);
                        if (it == tileMeshes.end()) continue;

                        auto& memMesh = it->second;
                        auto verts = memMesh.vertices; auto inds = memMesh.indices;

                        glm::mat4 RMat = glm::mat4(glm::rotate(glm::mat4(1.f), glm::radians(90.f * (float)r), glm::vec3(0, 1, 0)));

                        if (takeSymetry)
                        {
                            RMat = glm::translate(RMat, glm::vec3(0, 1, 0));
                            RMat = glm::scale(RMat, glm::vec3(1, -1, 1));

                            //The winding order of the triangles must be reversed if we took the symetry of the mesh
                            int triangleCount = inds.size() / 3;
                            for (int Tr = 0; Tr < triangleCount; Tr++)
                            {
                                uint i0 = inds[Tr * 3];
                                inds[Tr * 3] = inds[Tr * 3 + 2];
                                inds[Tr * 3 + 2] = i0;
                            }
                        }

                        for (auto& v : verts)
                        {
                            v.position = glm::vec3(RMat * glm::vec4(v.position, 1.f));
                            v.normal = glm::normalize(glm::vec3(RMat * glm::vec4(v.normal, 0.f)));
                        }

                        tileMeshes.emplace(B, std::move(MemMesh(verts, inds)));
                        assignedMesh = true;
                        break;
                    }
                }
            }
        }
        else if (version == 2)
        {
            //Base Tile Bytes
            static std::unordered_set<uint> premades{
                0x00, 0x01, 0x03,// 0x05, 0x07, 0x0f,
                0x10, 0x11, 0x13,// 0x15, 0x17, 0x1f
            };
            static const char hexChars[] = "0123456789abcdef";
            ModelImport loadModel;

            std::string buffer;
            buffer.reserve(64);

            //Importing base Tiles
            for (uint b : premades)
            {
                buffer.clear();
                buffer += "assets/3dtilesv2/";

                if (b > 0xFF) buffer += hexChars[b >> 8];
                if (b > 0x0F) buffer += hexChars[(b >> 4) & 0x0F];
                buffer += hexChars[b & 0x0F];
                buffer += ".obj";

                loadModel.ImportPath(buffer, false);
                if (!loadModel.fullImport) continue;
                auto& mesh = loadModel.meshes[0];
                const auto& impVerts = mesh.verts;
                const auto& impInds = mesh.inds;

                //Seperaing triangles
                vector<Vertex> verts; verts.reserve(impInds.size());
                vector<uint> inds; inds.reserve(impInds.size());

                auto triangleCount = impInds.size() / 3;
                for (uint t = 0; t < triangleCount; t++)
                {
                    inds.emplace_back(t * 3); inds.emplace_back(t * 3 + 1); inds.emplace_back(t * 3 + 2);
                    int vi[3] = { t * 3, t * 3 + 1, t * 3 + 2 };
                    for (int v = 0; v < 3; v++)
                    {
                        uint impInd = impInds[vi[v]];
                        verts.emplace_back(mesh.verts[impInd], mesh.norms[impInd], mesh.uvs[impInd]);
                    }

                    glm::vec3 normal = glm::normalize(glm::cross(verts[vi[1]].position - verts[vi[0]].position, verts[vi[2]].position - verts[vi[0]].position));
                    verts[vi[0]].normal = normal; verts[vi[1]].normal = normal; verts[vi[2]].normal = normal;
                }

                tileMeshes.emplace(b, std::move(MemMesh(verts, inds)));

            }

            //Creating the other Tile rotations and vertical symetries
            static auto rotateBits = [](uint b) -> uint
                {
                    uint shell = ((b << 1) | ((b >> 3) & 1u)) & 0xFu;
                    return shell | (b & (3u << 4));
                };

            static auto flipCovers = [](uint b) -> uint
                {
                    uint x = ((b >> 4) ^ (b >> 5)) & 1u;
                    return b ^ (x << 4) ^ (x << 5);
                };

            for (uint B = 1; B < 0x30; B++)
            {
                bool assignedMesh = false;
                for (int s = 0; !assignedMesh && s < 2; s++) //if no rotations exists, look for vertical symetries
                {
                    bool takeSymetry = s;
                    if (tileMeshes.contains(B)) { assignedMesh = true; break; }

                    uint rB = takeSymetry ? flipCovers(B) : B;
                    for (int r = 0; r < 4; r++)
                    {
                        auto it = tileMeshes.find(rB);
                        rB = rotateBits(rB);
                        if (it == tileMeshes.end()) continue;

                        auto& memMesh = it->second;
                        auto verts = memMesh.vertices; auto inds = memMesh.indices;

                        glm::mat4 RMat = glm::mat4(glm::rotate(glm::mat4(1.f), glm::radians(90.f * (float)r), glm::vec3(0, 1, 0)));

                        if (takeSymetry)
                        {
                            RMat = glm::translate(RMat, glm::vec3(0, 1, 0));
                            RMat = glm::scale(RMat, glm::vec3(1, -1, 1));

                            //The winding order of the triangles must be reversed if we took the symetry of the mesh
                            int triangleCount = inds.size() / 3;
                            for (int Tr = 0; Tr < triangleCount; Tr++)
                            {
                                uint i0 = inds[Tr * 3];
                                inds[Tr * 3] = inds[Tr * 3 + 2];
                                inds[Tr * 3 + 2] = i0;
                            }
                        }

                        for (auto& v : verts)
                        {
                            v.position = glm::vec3(RMat * glm::vec4(v.position, 1.f));
                            v.normal = glm::normalize(glm::vec3(RMat * glm::vec4(v.normal, 0.f)));
                        }

                        tileMeshes.emplace(B, std::move(MemMesh(verts, inds)));
                        assignedMesh = true;
                        break;
                    }
                }
            }
        }

        //Mapping UVs
        static const float threshold = 0.708;
        for (auto& pair : tileMeshes)
        {
            auto& vertices = pair.second.vertices;
            auto& indices = pair.second.indices;
            auto triangleCount = indices.size() / 3;
            for (int t = 0; t < triangleCount; t++)
            {
                auto normal = vertices[indices[t * 3]].normal;
                auto absNormal = abs(normal);

                if (absNormal.y > absNormal.x && absNormal.y > absNormal.z) {
                    // Normal points up/down - use XZ plane
                    for (int i = 0; i < 3; i++)
                    {
                        auto& v = vertices[indices[t * 3 + i]];
                        v.texCoords = glm::vec2(glm::sign(normal.y) * (v.position.x + 0.5f), -v.position.z - 0.5f);
                    }
                }
                else if (absNormal.x > absNormal.z) {
                    // Normal points left/right - use YZ plane
                    for (int i = 0; i < 3; i++)
                    {
                        auto& v = vertices[indices[t * 3 + i]];
                        v.texCoords = glm::vec2(glm::sign(normal.x) * (-v.position.z - 0.5f), v.position.y);
                    }
                }
                else {
                    // Normal points forward/back - use XY plane
                    for (int i = 0; i < 3; i++)
                    {
                        auto& v = vertices[indices[t * 3 + i]];
                        v.texCoords = glm::vec2(glm::sign(normal.z) * v.position.x + 0.5f, v.position.y);
                    }
                }
                /*
                if (absNormal.y > threshold)
                {
                    for (int i = 0; i < 3; i++)
                    {
                        auto& v = vertices[indices[t * 3 + i]];
                        v.texCoords = glm::vec2(v.position.x + 0.5f, - v.position.z - 0.5f);
                    }
                }
                else
                {
                    for (int i = 0; i < 3; i++)
                    {
                        auto& v = vertices[indices[t * 3 + i]];
                        float angle = atan2(v.position.z, v.position.x);
                        float u = (angle + 2*glm::pi<float>()) / (2.0f * glm::pi<float>());
                        float vCoord = v.position.y;
                        v.texCoords = glm::vec2(u, vCoord);
                    }
                }*/
                //glm::vec2 minTC = glm::min(vertices[indices[t * 3]].texCoords, vertices[indices[t * 3 + 1]].texCoords);
                //minTC = floor(glm::min(minTC, vertices[indices[t * 3 + 2]].texCoords));
                //for (int i = 0; i < 3; i++) { vertices[indices[t * 3 + i]].texCoords -= minTC; }
            }
        }


    }

}