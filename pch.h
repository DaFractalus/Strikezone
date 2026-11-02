#ifndef PCH_HPP
#define PCH_HPP

//Precompiled header file
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <stb/stb_image_resize2.h>
#include <stb/stb_image.h>
#define STB_VORBIS_HEADER_ONLY
#include <stb/stb_vorbis.c>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <unordered_set>
#include <shared_mutex>
#include <filesystem>
#include <functional>
#include <typeindex>
#include <execution>
#include <iostream>
#include <variant>
#include <fstream>
#include <sstream>
#include <random>
#include <chrono>
#include <thread>
#include <memory>
#include <vector>
#include <mutex>
#include <stack>
#include <queue>
#include <cmath>
#include <array>
#include <any>
#include <map>

using std::vector, std::min, std::max;
typedef uint8_t Byte;
typedef uint32_t uint;
typedef std::ostringstream OSS;

#endif