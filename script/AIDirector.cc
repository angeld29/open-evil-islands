#include "AIDirector.h"

namespace cursedearth
{

    template<class T>
    void AIDirector<T>::GSSetVar(double player, std::string* variable, double value)
    {
    }

    template<class T>
    void AIDirector<T>::GSSetVarMax(double player, std::string* variable, double value)
    {
    }

    template<class T>
    double AIDirector<T>::GSGetVar(double player, std::string* variable)
    {
        return 1;
    }

    template<class T>
    void AIDirector<T>::GSDelVar(double player, std::string* variable)
    {
    }

    template class AIDirector<>;
}
