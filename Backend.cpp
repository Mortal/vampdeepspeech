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
		if (r == -1) throw vds::syscall_failed("dup2");
		r = dup2(m_fromProcess.write_end(), 1);
		if (r == -1) throw vds::syscall_failed("dup2");
		execl(filename, filename, nullptr);
		std::cerr << "Could not run " << filename << std::endl;
		_exit(123);
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
}

namespace vds {

class Backend::impl {
public:
    impl(std::unique_ptr<Process> process) : m_proc(std::move(process)) {
	m_hasData = false;
	m_results = fdopen(m_proc->from_process(), "r");
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

    std::string getline() {
	char * lineptr = nullptr;
	size_t n = 0;
	ssize_t r = ::getline(&lineptr, &n, m_results);
	if (r == -1) {
	    if (lineptr != nullptr) ::free(lineptr);
	    throw vds::syscall_failed("getline");
	}
	while (r > 0 && lineptr[r-1] == '\n') r--;
	std::string line(lineptr, lineptr+r);
	::free(lineptr);
	return line;
    }

    std::string infer() {
	if (!m_hasData) return "";
	feed(nullptr, 0);
	std::string res = getline();
	m_hasData = false;
	return res;
    }

    void clear() {
	infer();
    }

private:
    std::unique_ptr<Process> m_proc;
    FILE * m_results;
    bool m_hasData;
};

std::unique_ptr<Backend>
Backend::make(std::string filename) {
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
