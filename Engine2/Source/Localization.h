#pragma once
#include <unordered_map>
#include <string>

namespace E2
{
    struct Token
    {
        std::string token;
        std::string replacement;
    };

    struct Filter
    {
        std::string id;
        void(*pFilter)(std::string&);
    };

    class Localization
    {
        std::unordered_map<std::string, std::string> m_tokenValueMap;
        std::unordered_map<std::string, void(*)(std::string&)> m_filterCallbackMap;

    public:
        void LoadToken(const Token& token);
        void LoadFilterCallback(const Filter& filter);
        void Substitute(std::string& string);

    private:
        void ReplaceToken(std::string& token, std::string& filter);
    };
}