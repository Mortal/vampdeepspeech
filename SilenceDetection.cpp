#include "SilenceDetection.h"
#include <cmath>

namespace vds {

class SilenceDetection::impl {
public:
    bool silent(const float * chunk, size_t chunkSize) {
	for (size_t i = 0; i < chunkSize; ++i)
	    if (fabsf(chunk[i]) > 0.005)
		return false;
	return true;
    }
};

SilenceDetection::SilenceDetection()
    : pimpl(new SilenceDetection::impl()) {}
SilenceDetection::~SilenceDetection() {}
bool SilenceDetection::silent(const float * chunk, size_t chunkSize) {
    return pimpl->silent(chunk, chunkSize);
}

}
