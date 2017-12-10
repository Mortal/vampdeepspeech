#include <memory>

namespace vds {
    class Backend {
    public:
	Backend(std::string filename);
	~Backend();
	void feed(const float * chunk, size_t n);
	std::string infer();
    private:
	class impl;
	std::unique_ptr<impl> pimpl;
    };
}
