// vim:set sw=4 noet:
#include "Backend.h"
#include "vds_error.h"
#include <deepspeech.h>
#include <vector>
#include <iostream>
#include <time.h>

#define N_CEP 26
#define N_CONTEXT 9
#define BEAM_WIDTH 500
#define LM_WEIGHT 1.75f
#define WORD_COUNT_WEIGHT 1.00f
#define VALID_WORD_COUNT_WEIGHT 1.00f

namespace {
    class timer {
    public:
	timer() {
	    ::clock_gettime(CLOCK_MONOTONIC, &t1);
	}

	double elapsed() {
	    ::clock_gettime(CLOCK_MONOTONIC, &t2);
	    double res = (t2.tv_sec-t1.tv_sec) + 1e-9 * (t2.tv_nsec - t1.tv_nsec);
	    t1 = t2;
	    return res;
	}
    private:
	struct timespec t1;
	struct timespec t2;
    };

    std::unique_ptr<DeepSpeech::Model>
    get_model(std::string dirname) {
	std::string modelPath = dirname + "/models/output_graph.pb";
	std::string alphabetPath = dirname + "/models/alphabet.txt";
	std::string lmPath = dirname + "/models/lm.binary";
	std::string triePath = dirname + "/models/trie";
	timer t;
	std::unique_ptr<DeepSpeech::Model> ctx =
	    std::make_unique<DeepSpeech::Model>(modelPath.c_str(), N_CEP, N_CONTEXT, alphabetPath.c_str(), BEAM_WIDTH);
	std::cerr << "Loading model " << modelPath << " took " << t.elapsed() << " s" << std::endl;
	ctx->enableDecoderWithLM(alphabetPath.c_str(), lmPath.c_str(), triePath.c_str(), LM_WEIGHT, WORD_COUNT_WEIGHT, VALID_WORD_COUNT_WEIGHT);
	std::cerr << "Loading language model " << lmPath << " took " << t.elapsed() << " s" << std::endl;
	return ctx;
    }
}

#define SAMPLE_RATE (16000)

namespace vds {

class Backend::impl {
public:
    impl(std::unique_ptr<DeepSpeech::Model> ctx)
	: m_ctx(std::move(ctx))
    {
    }

    void feed(const float * chunk, size_t n) {
	size_t a = m_buf.size();
	m_buf.resize(a + n);
	for (size_t i = 0; i < n; ++i)
	    m_buf[a+i] = chunk[i] * (1<<15);
    }

    std::string infer() {
	if (m_buf.empty()) return "";
	timer t;
	char * r = m_ctx->stt(&m_buf[0], m_buf.size(), SAMPLE_RATE);
	if (r == nullptr)
	    throw vds::syscall_failed("Model::stt");
	std::string res = r;
	std::cerr << "[" << t.elapsed() << " s/"
	    << (m_buf.size() / 16000.0) << " s] \""
	    << res << "\"" << std::endl;
	::free(r);
	m_buf.clear();
	return res;
    }

    void clear() {
	m_buf.clear();
    }

private:
    std::vector<short> m_buf;
    std::unique_ptr<DeepSpeech::Model> m_ctx;
};

std::unique_ptr<Backend>
Backend::make(std::string dirname) {
    return std::make_unique<Backend>(
	std::make_unique<Backend::impl>(
	    get_model(dirname)));
}

Backend::Backend(std::unique_ptr<Backend::impl> pimpl)
    : pimpl(std::move(pimpl)) {}

Backend::~Backend() {}
void Backend::feed(const float * chunk, size_t n) {pimpl->feed(chunk, n);}
std::string Backend::infer() {return pimpl->infer();}
void Backend::clear() {pimpl->clear();}

}
