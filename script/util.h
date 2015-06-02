#ifndef UTIL_H
#define UTIL_H

#include <string>

namespace EIScript
{

    struct CaseInsensitiveHash {
        std::size_t operator()(const std::string& Keyval) const {

            std::size_t hash = 5381;
            std::for_each(Keyval.begin() , Keyval.end() , [&](char c) {
                hash += ((hash << 5) + hash) + std::tolower(c);
            });
            return hash;
        }
    };

    struct CaseInsensitiveEqual {
        bool operator()(const std::string& Left, const std::string& Right) const {
            return Left.size() == Right.size()
                   && std::equal(Left.begin(), Left.end(), Right.begin(),
            [](char a , char b) {
                return std::tolower(a) == std::tolower(b);
            }
                                );
        }
    };

}

#endif // UTIL_H
