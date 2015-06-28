#pragma once

#include "EIScriptDriver.hpp"
    
namespace EIScript {

	class Parser : public BisonParser {
        friend class Driver;
		public:
			Parser(Driver& driver) : BisonParser(driver) {}
	};
}
