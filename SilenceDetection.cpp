// vim:set sw=4 noet:
#include "SilenceDetection.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <random>
#include <iostream>

template <typename Compare>
void heapreplace(float * heap, size_t target, size_t size, float value, Compare comp) {
    heap[target] = heap[0];
    std::push_heap(heap, heap + (target+1), comp);
    // Now, [heap, heap+size) is again a heap
    std::pop_heap(heap, heap + size, comp);
    heap[size-1] = value;
    std::push_heap(heap, heap + size, comp);
}

class Fractile {
public:
    Fractile(double f, size_t n)
	: k(f*n), heaps(n), n_smaller(0), N(0)
    {
    }

    void add(float x) {
	++N;
	// Skip element with probability 1 - n/N
	if (rng() % N > heaps.size()) return;

	if (N == 1) {
	    heaps[0] = x;
	    n_smaller = 1;
	} else if (N > heaps.size()) {
	    if (x < heaps[0]) {
		// x belongs in smaller group
		heapreplace(&heaps[0], rng() % k, k, x, std::less<float>());
	    } else {
		size_t n = heaps.size();
		heapreplace(&heaps[0]+k, rng() % (n-k), n-k, x, std::greater<float>());
	    }
	} else {
	    if (x < heaps[0]) {
		// x belongs in smaller group
		if (n_smaller == k) move_to_larger();
		heaps[n_smaller] = x;
		std::push_heap(&heaps[0], &heaps[++n_smaller], std::less<float>());
	    } else {
		if (k + N - n_smaller - 1 == heaps.size()) move_to_smaller();
		heaps[k + N - n_smaller - 1] = x;
		std::push_heap(&heaps[k], &heaps[k] + (N - n_smaller),
			std::greater<float>());
	    }
	    // Target: k / heaps.size()
	    // Current: n_smaller / N
	    // n_smaller = k * N / heaps.size()
	    while (n_smaller < k * N / heaps.size()) {
		move_to_smaller();
	    }
	    while (n_smaller > 1 && n_smaller > k * N / heaps.size()) {
		move_to_larger();
	    }
	}
	// std::cerr << "Smaller: " << n_smaller << " Total: " << N
	//     << " " << heaps[0] << " " << heaps[k]
	//     << std::endl;
    }

    void move_to_larger() {
	size_t n = std::min(N, heaps.size());
	size_t n_larger = n - n_smaller;
	heaps[k + n_larger] = heaps[0];
	std::pop_heap(&heaps[0], &heaps[n_smaller--], std::less<float>());
	std::push_heap(&heaps[k], &heaps[k] + (n_larger+1), std::greater<float>());
    }

    void move_to_smaller() {
	size_t n = std::min(N, heaps.size());
	size_t n_larger = n - n_smaller;
	heaps[n_smaller] = heaps[k];
	std::push_heap(&heaps[0], &heaps[++n_smaller], std::less<float>());
	std::pop_heap(&heaps[k], &heaps[k] + n_larger, std::greater<float>());
    }

    size_t samples() const { return N; }

    float estimate() {
	return heaps[0];
    }

private:
    size_t k;
    std::vector<float> heaps;
    size_t n_smaller;
    size_t N;
    std::mt19937 rng;
};

namespace vds {

class SilenceDetection::impl {
public:
    impl() : m_fractile(1.0 / 16, 1024) {}

    bool silent(const float * chunk, size_t chunkSize) {
	float sum = 0.0f;
	for (size_t i = 0; i < chunkSize; ++i) sum += fabsf(chunk[i]);
	float mean = sum / chunkSize;
	m_fractile.add(mean);
	float threshold = m_fractile.estimate();
	return mean < threshold;
    }

private:
    Fractile m_fractile;
};

SilenceDetection::SilenceDetection()
    : pimpl(new SilenceDetection::impl()) {}
SilenceDetection::~SilenceDetection() {}
bool SilenceDetection::silent(const float * chunk, size_t chunkSize) {
    return pimpl->silent(chunk, chunkSize);
}

}
