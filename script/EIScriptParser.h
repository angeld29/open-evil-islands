#pragma once

namespace EIScript {
	class Parser {
		public:
			Parser() : parser(scanner) {}
		
			int parse() {
				return parser.parse();
			}
		
		private:
			EIScript::FlexScanner scanner;
			EIScript::BisonParser parser;
	};
}
