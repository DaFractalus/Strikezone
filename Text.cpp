#include "Text.hpp"

namespace TLB
{
    Language::Language() : path() {}

    bool Language::LoadLanguage(const std::string& filename)
    {
        path = filename;

        std::ifstream file(filename);
        if (!file.is_open()) return false;

        nlohmann::json j;
        try { file >> j; }
        catch (const std::exception& e)
        {
            std::cerr << "[Language] Failed to parse JSON: " << e.what() << "\n";
            return false;
        }

        if (!j.is_object())
        {
            std::cerr << "[Language] Invalid format: root must be an object.\n";
            return false;
        }

        translations.clear();
        for (auto& [key, value] : j.items())
        {
            if (value.is_string()) { translations[key] = value.get<std::string>(); }
            else { std::cerr << "[Language] Warning: key '" << key << "' is not a string, skipping.\n"; }
        }

        return true;
    }


    const std::string& Language::Get(const std::string& key) const
    {
        auto it = translations.find(key);
        return (it != translations.end()) ? it->second : key;
    }
}