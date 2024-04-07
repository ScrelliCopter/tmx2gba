/*********************************************************************
Matt Marchant 2016 - 2021
http://trederia.blogspot.com

tmxlite - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#pragma once

#include "tmxlite/detail/Log.hpp"
#include "tmxlite/Types.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <algorithm>

namespace tmx
{
    //using inline here just to supress unused warnings on gcc
    bool decompress(const char* source, std::vector<unsigned char>& dest, std::size_t inSize, std::size_t expectedSize);

    static inline Colour colourFromString(std::string str)
    {
        //removes preceding #
        auto result = str.find_last_of('#');
        if (result != std::string::npos)
        {
            str = str.substr(result + 1);
        }

        if (str.size() == 6 || str.size() == 8)
        {
            unsigned int value, r, g, b;
            unsigned int a = 255;
            std::stringstream input(str);
            input >> std::hex >> value;

            r = (value >> 16) & 0xff;
            g = (value >> 8) & 0xff;
            b = value & 0xff;

            if (str.size() == 8)
            {
                a = (value >> 24) & 0xff;
            }

            return{ std::uint8_t(r), std::uint8_t(g), std::uint8_t(b), std::uint8_t(a) };
        }
        Logger::log(str + ": not a valid colour string", Logger::Type::Error);
        return{};
    }

    static inline std::string resolveFilePath(std::string path, const std::string& workingDir)
    {
        static const std::string match("../");
        std::size_t result = path.find(match);
        std::size_t count = 0;
        while (result != std::string::npos)
        {
            count++;
            path = path.substr(result + match.size());
            result = path.find(match);
        }

        if (workingDir.empty()) return path;

        std::string outPath = workingDir;
        for (auto i = 0u; i < count; ++i)
        {
            result = outPath.find_last_of('/');
            if (result != std::string::npos)
            {
                outPath = outPath.substr(0, result);
            }
        }

        return outPath + '/' + path;
    }

    static inline std::string getFilePath(const std::string& path)
    {
        //TODO this doesn't actually check that there is a file at the
        //end of the path, or that it's even a valid path...

        static auto searchFunc = [](const char separator, const std::string& path)->std::string
        {
            std::size_t i = path.rfind(separator, path.length());
            if (i != std::string::npos)
            {
                return(path.substr(0, i + 1));
            }

            return "";
        };


#ifdef _WIN32 //try windows formatted paths first
        std::string retVal = searchFunc('\\', path);
        if (!retVal.empty()) return retVal;
#endif

        return searchFunc('/', path);
    }
} //namespacec tmx
