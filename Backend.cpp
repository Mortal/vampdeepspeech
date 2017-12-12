// vim:set sw=4 noet:
#include "Backend.h"
#include "vds_error.h"
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <sstream>

namespace {
    class Pipe {
    public:
	Pipe() {
	    int fds[2];
	    int r = pipe(fds);
	    if (r == -1) throw vds::syscall_failed("pipe");
	    m_rd = fds[0];
	    m_wr = fds[1];
	}
	Pipe(const Pipe &) = delete;

	~Pipe() {
	    close();
	}

	void close() {
	    if (m_wr != -1) ::close(m_wr);
	    if (m_rd != -1) ::close(m_rd);
	    m_rd = m_wr = -1;
	}

	int read_end() {
	    if (m_wr != -1) ::close(m_wr);
	    m_wr = -1;
	    return m_rd;
	}

	int write_end() {
	    if (m_rd != -1) ::close(m_rd);
	    m_rd = -1;
	    return m_wr;
	}

    private:
	int m_rd, m_wr;
    };

    class Process {
    public:
	Process(const Process &) = delete;

	Process(const char * filename) {
	    int r = fork();
	    if (r == -1) throw vds::syscall_failed("fork");
	    if (r == 0) {
		r = dup2(m_toProcess.read_end(), 0);
		if (r == -1) {
		    std::cout << "dup2 failed (1): "
			<< ::strerror(errno) << std::endl;
		    _exit(0);
		}
		r = dup2(m_fromProcess.write_end(), 1);
		if (r == -1) {
		    std::cout << "dup2 failed (2): "
			<< ::strerror(errno) << std::endl;
		    _exit(0);
		}
		execl(filename, filename, nullptr);
		std::cout << "Could not run " << filename
		    << ": " << ::strerror(errno) << std::endl;
		_exit(0);
	    }
	    m_toProcess.write_end();
	    m_fromProcess.read_end();
	    m_pid = r;
	}

	~Process() {
	    close();
	    if (m_pid != -1) waitpid(m_pid, 0, 0);
	    m_pid = -1;
	}

	int to_process() { return m_toProcess.write_end(); }
	int from_process() { return m_fromProcess.read_end(); }
	void write_done() {
	    m_toProcess.close();
	}
	void read_done() {
	    m_fromProcess.close();
	}
	void close() {
	    m_toProcess.close();
	    m_fromProcess.close();
	}

    private:
	Pipe m_toProcess;
	Pipe m_fromProcess;
	int m_pid;
    };

    class LineReader {
    public:
	LineReader(int fd)
	    : m_file(::fdopen(fd, "r"))
	    , m_buf(nullptr)
	    , m_bufSize(0)
	{
	    if (m_file == nullptr) throw vds::syscall_failed("fdopen");
	}

	~LineReader() {
	    ::free(m_buf);
	    m_buf = nullptr;
	    if (m_file != nullptr) ::fclose(m_file);
	    m_file = nullptr;
	}

	LineReader(const LineReader &) = delete;

	std::string next() {
	    ssize_t r = ::getline(&m_buf, &m_bufSize, m_file);
	    if (r == -1)
		throw vds::syscall_failed("getline");
	    while (r > 0 && m_buf[r-1] == '\n') r--;
	    return std::string(m_buf, m_buf + r);
	}

    private:
	FILE * m_file;
	char * m_buf;
	size_t m_bufSize;
    };
}

namespace vds {

class Backend::impl {
public:
    impl(std::unique_ptr<Process> process)
	: m_proc(std::move(process))
	, m_results(std::make_unique<LineReader>(m_proc->from_process()))
	, m_hasData(false)
    {
	std::string line = m_results->next();
	if (line != "OK") {
	    throw vds::configuration_error(line);
	}
    }

    ~impl() {
	clear();
	feed(nullptr, 0);
    }

    void feed(const float * chunk, size_t n) {
	int fd = m_proc->to_process();
	int written = 0;
	int nbytes = n*sizeof(float);
	char * buf = (char *) chunk;
	if (write(fd, &n, 4) != 4) throw vds::syscall_failed("write 1");
	while (written < nbytes) {
	    int r = write(fd, buf+written, nbytes - written);
	    if (r < 0) throw vds::syscall_failed("write 2");
	    written += r;
	}
	m_hasData = true;
    }

    std::string infer() {
	if (!m_hasData) return "";
	feed(nullptr, 0);
	std::string res = m_results->next();
	m_hasData = false;
	return res;
    }

    void clear() {
	infer();
    }

private:
    std::unique_ptr<Process> m_proc;
    std::unique_ptr<LineReader> m_results;
    bool m_hasData;
};

std::unique_ptr<Backend>
Backend::make(std::string dirname) {
    std::string filename = dirname + "/deepspeechdirect.py";
    return std::make_unique<Backend>(
	std::make_unique<Backend::impl>(
	    std::make_unique<Process>(filename.c_str())));
}

Backend::Backend(std::unique_ptr<Backend::impl> pimpl)
    : pimpl(std::move(pimpl)) {}

Backend::~Backend() {}
void Backend::feed(const float * chunk, size_t n) {pimpl->feed(chunk, n);}
std::string Backend::infer() {return pimpl->infer();}
void Backend::clear() {pimpl->clear();}

}
