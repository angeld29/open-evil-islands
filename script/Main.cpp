#include "EIScriptScanner.h"
#include "EIScriptParser.h"

// Entry Point
int main(int argc, char * argv[]) {
	EIScript::Parser parser;
	return parser.parse();
}

