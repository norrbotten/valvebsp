#include <iostream>

#include "parser.h"

int main(int argc, char** argv) {
	auto err = []() {
		std::cerr << "Usage: [-v] filename\n -v : Verbose mode\n";
	};

	if (argc < 2) {
		err();
		return 1;
	}

	// Parse arguments
	std::string fn(argv[1]);
	bool verbose = false;

	if (fn == "-v") {
		if (argc < 3) {
			err();
			return 1;
		}
		else {
			verbose = true;
			fn = std::string(argv[2]);
		}
	}

	ValveBSP::Parser parser(fn, verbose);

	if (!parser.IsReady()) {
		return 1;
	}

	if (!parser.Parse()) {
		std::cerr << "Parsing error! No file generated.\n";
		return 1;
	}

	return 0;
}