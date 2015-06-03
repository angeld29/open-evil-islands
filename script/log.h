#ifndef EI_BREVNO
#define EI_BREVNO

#include <iostream>
#include <string>

namespace EIScript
{

    class log
    {
    public:
        static bool const debug = true; //TODO this shouldn't be const, but oh bother
        static bool const verbose = true;

        static write(std::string& message) {
            std::cout<<message<<std::endl;
        }

        static d(std::string& message) {
            if(debug) {
                std::cout<<message<<std::endl;
            }
        }

        static d(std::string& tag, std::string& message) {
            if(debug) {
                std::cout<<tag<<" "<<message<<std::endl;
            }
        }

        static v(std::string& message) {
            if(verbose) {
                std::cout<<message<<std::endl;
            }
        }

        static v(std::string& tag, std::string& message) {
            if(verbose) {
                std::cout<<tag<<" "<<message<<std::endl;
            }
        }

    private:
        log();
        ~log();

    };

}

#endif // EI_BREVNO
