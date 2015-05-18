#pragma once

#include "EIScriptDriver.h"
    
namespace EIScript {
	class Parser {
		public:
			Parser(Driver& driver) : parser(driver) {}
		
			int parse() {
				return parser.parse();
			}
		
		private:
			EIScript::BisonParser parser;
	};
}
