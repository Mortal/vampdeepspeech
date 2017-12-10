#include <memory>

namespace vds {
    class SilenceDetection {
    public:
	SilenceDetection();
	~SilenceDetection();
	bool silent(const float * chunk, size_t chunkSize);
    private:
	class impl;
	std::unique_ptr<impl> pimpl;
    };
}
