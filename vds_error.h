// vim:set sw=4 noet:
#ifndef VDS_ERROR_H
#define VDS_ERROR_H
#include <iostream>
#include <stdexcept>

namespace vds {
    class exception : public std::runtime_error {
    public:
	using std::runtime_error::runtime_error;
    };
    class configuration_error : public exception {
    public:
	using exception::exception;
    };
    class syscall_failed : public exception {
    public:
	using exception::exception;
    };
}

#endif /* VDS_ERROR_H */
