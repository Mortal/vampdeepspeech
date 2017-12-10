#include <iostream>
#include <stdexcept>

namespace {
    void vds_error(const char * s) {
	std::cerr << s << std::endl;
	throw std::runtime_error(s);
    }
}
