#include "pch.h"
#include "Localization.h"
#include <cctype>

void E2::Localization::LoadToken(const Token& token)
{
    m_tokenValueMap.emplace(token.token, token.replacement);
}

void E2::Localization::LoadFilterCallback(const Filter& filter)
{
    m_filterCallbackMap.emplace(filter.id, filter.pFilter);
}

void E2::Localization::Substitute(std::string& string)
{
    size_t currentIndex = 0;
    while (currentIndex != string.size())
    {
        if (string[currentIndex] == '{')
        {
            size_t replaceBeginIndex = currentIndex;
            size_t replaceEndIndex = string.find('}', currentIndex);

            if (replaceBeginIndex + 1 == replaceEndIndex)
            {
                //asd{}asd
                // just remove it
                ++currentIndex;
                continue;
            }

            if (replaceEndIndex == std::string::npos)
            {
                // no }
                return;
            }

            auto tokenStr = string.substr(replaceBeginIndex+1, replaceEndIndex - replaceBeginIndex -1);
            size_t pipeIndex = tokenStr.find('|');
            std::string token;
            std::string filter;

            if (pipeIndex == 0)
            {
                // first char is |

            }
            else if (pipeIndex == string.size() - 1)
            {
                // last char is |
            }
            else if (pipeIndex == std::string::npos)
            {
                // no |
                token = tokenStr;
            }
            else 
            {
                token = tokenStr.substr(0, pipeIndex);
                filter = tokenStr.substr(pipeIndex + 1, tokenStr.size() - 1);
            }

            ReplaceToken(token, filter);
            string.replace(currentIndex, replaceEndIndex - currentIndex +1 , token);
            replaceEndIndex = currentIndex + token.size() -1;
            currentIndex = replaceEndIndex;
        }
        ++currentIndex;
    }
}

void E2::Localization::ReplaceToken(std::string& token, std::string& filter)
{
    // invalid token
    if (m_tokenValueMap.find(token) != m_tokenValueMap.end())
    {
        token = m_tokenValueMap[token];
    }

    // invalid filter
    if (filter.size() != 0
        && m_filterCallbackMap.find(filter) != m_filterCallbackMap.end())
    {
        m_filterCallbackMap[filter](token);
    }
}
