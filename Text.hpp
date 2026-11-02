#pragma once
#ifndef TEXT_HPP
#define TEXT_HPP

#include <nlohmann/json.hpp>

namespace TLB
{
    class Language
    {
        std::unordered_map<std::string, std::string> translations;

    public:
        std::string path;

        Language();
        bool LoadLanguage(const std::string& filename);
        const std::string& Get(const std::string& key) const;

    };

}

#endif