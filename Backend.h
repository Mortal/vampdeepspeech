// vim:set sw=4 noet:
#include <memory>

namespace vds {
    class Backend {
    private:
	class impl;
	std::unique_ptr<impl> pimpl;
    public:
	Backend(std::unique_ptr<impl> pimpl);
	static std::unique_ptr<Backend> make(std::string dirname);
	~Backend();
	void feed(const float * chunk, size_t n);
	void clear();
	std::string infer();
    };
}
