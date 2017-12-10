#include "Backend.h"
#include "vds_error.h"
#include <unistd.h>
#include <sys/wait.h>

namespace {
    class Pipe {
    public:
	Pipe() {
	    int fds[2];
	    int r = pipe(fds);
	    if (r == -1) vds_error("pipe");
	    m_rd = fds[0];
	    m_wr = fds[1];
	}

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
	Process(const char * filename) {
	    int r = fork();
	    if (r == -1) vds_error("fork");
	    if (r == 0) {
		r = dup2(m_toProcess.read_end(), 0);
		if (r == -1) vds_error("dup2");
		r = dup2(m_fromProcess.write_end(), 1);
		if (r == -1) vds_error("dup2");
		execl(filename, filename, nullptr);
		std::cerr << "Could not run " << filename << std::endl;
		_exit(123);
	    }
	    m_toProcess.write_end();
	    m_fromProcess.read_end();
	    m_pid = r;
	}

	~Process() {
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

    private:
	Pipe m_toProcess;
	Pipe m_fromProcess;
	int m_pid;
    };
}

namespace vds {

class Backend::impl {
public:
    impl(std::string filename) : m_filename(filename) {
    }

    void launch() {
	if (m_proc) return;
	m_proc.reset(new Process(m_filename.c_str()));
	m_results = fdopen(m_proc->from_process(), "r");
    }

    void feed(float * chunk, size_t n) {
	launch();
	int fd = m_proc->to_process();
	write(fd, &n, 4);
	write(fd, chunk, n*sizeof(float));
    }

    std::string infer() {
	write(m_proc->to_process(), "\0\0\0\0", 4);
	char ** lineptr = 0;
	size_t n = 0;
	ssize_t r = ::getline(lineptr, &n, m_results);
	if (r == -1) {
	    ::free(*lineptr);
	    vds_error("getline");
	}
	std::string line = *lineptr;
	::free(*lineptr);
	return line;
    }

private:
    std::string m_filename;
    std::unique_ptr<Process> m_proc;
    FILE * m_results;
};

Backend::Backend(std::string filename)
    : pimpl(new Backend::impl(filename)) {}

Backend::~Backend() {}
void Backend::feed(float * chunk, size_t n) {pimpl->feed(chunk, n);}
std::string Backend::infer() {return pimpl->infer();}

}
