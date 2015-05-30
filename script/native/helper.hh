#ifndef HELPER_H__
#define HELPER_H__

#include <type_traits>

namespace EIScript
{
    namespace Helper
    {
        /* Nice but currently useless code start */
        template<unsigned int>
        struct from;

        template<>
        struct from<0> {
            template<typename type>
            static type string(const std::string& s) {return (std::stold(s));}
        };

        template<>
        struct from<1> {
            template<typename type>
            static type string(const std::string& s) {return (std::stoull(s));}
        };

        template<>
        struct from<2> {
            template<typename type>
            static type string(const std::string& s) {return (std::stoll(s));}
        };

        template<typename type>
        type FromString(const std::string& s)
        {
            return (from<0
                    + (!std::is_floating_point<type>::value ?
                       (std::is_signed<type>::value == true) : (0))
                    + (std::is_integral<type>::value == true)>::template string<type>(s));
        }
        /* end */


        template <typename _Get_TypeName>
        const char* getTypeName()
        {
            static std::string name;

            if(name.empty()) {
                const char *beginStr = "_Get_TypeName =";
                const size_t beginStrLen = 15;
                size_t begin,length;
                name = __PRETTY_FUNCTION__;

                begin = name.find(beginStr) + beginStrLen + 1;
                length = name.find("]",begin) - begin;
                name = name.substr(begin,length);
            }

            return name.c_str();
        }


        template<typename type>
        struct TypePriorityFor {
            static unsigned int get() {
                return (1
                        + (!std::is_floating_point<type>::value ?
                           (std::is_signed<type>::value == true) : (0))
                        + (std::is_integral<type>::value == true)
                        + (std::is_same<type, std::string>::value == false)
                       );
            }
        };

    };
};

#endif
