#ifndef EI_BREVNO
#define EI_BREVNO

#include <iostream>
#include <string>

namespace EIScript
{

    class Log
    {
    public:
        static bool const debug = true; //TODO this shouldn't be const, but oh bother
        static bool const verbose = true;

        static void write(std::string& message) {
            std::cout<<message<<std::endl;
        }

        static void d(std::string& message) {
            if(debug) {
                std::cout<<message<<std::endl;
            }
        }

        static void d(std::string& tag, std::string& message) {
            if(debug) {
                std::cout<<tag<<" "<<message<<std::endl;
            }
        }

        static void v(std::string& message) {
            if(verbose) {
                std::cout<<message<<std::endl;
            }
        }

        static void v(std::string& tag, std::string& message) {
            if(verbose) {
                std::cout<<tag<<" "<<message<<std::endl;
            }
        }

    private:
        Log() {};
        ~Log() {};

    };

}

#endif // EI_BREVNO
