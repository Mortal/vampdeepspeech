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
    impl(std::string filename) : m_filename(filename) {
	m_hasData = false;
    }

    ~impl() {
	clear();
	feed(nullptr, 0);
    }

    void launch() {
	if (m_proc) return;
	m_proc.reset(new Process(m_filename.c_str()));
	m_hasData = false;
	m_results = fdopen(m_proc->from_process(), "r");
    }

    void feed(const float * chunk, size_t n) {
	launch();
	int fd = m_proc->to_process();
	int written = 0;
	int nbytes = n*sizeof(float);
	char * buf = (char *) chunk;
	write(fd, &n, 4);
	while (written < nbytes) {
	    written += write(fd, buf+written, nbytes - written);
	}
	m_hasData = true;
    }

    std::string infer() {
	if (!m_hasData) return "";
	feed(nullptr, 0);
	char * lineptr = nullptr;
	size_t n = 0;
	ssize_t r = ::getline(&lineptr, &n, m_results);
	m_hasData = false;
	if (r == -1) {
	    if (lineptr != nullptr) ::free(lineptr);
	    vds_error("getline");
	}
	std::string line = lineptr;
	::free(lineptr);
	while (line.size() > 0 && line[line.size()-1] == '\n')
	    line.resize(line.size()-1);
	return line;
    }

    void clear() {
	infer();
    }

private:
    std::string m_filename;
    std::unique_ptr<Process> m_proc;
    FILE * m_results;
    bool m_hasData;
};

Backend::Backend(std::string filename)
    : pimpl(new Backend::impl(filename)) {}

Backend::~Backend() {}
void Backend::feed(const float * chunk, size_t n) {pimpl->feed(chunk, n);}
std::string Backend::infer() {return pimpl->infer();}
void Backend::clear() {pimpl->clear();}

}
