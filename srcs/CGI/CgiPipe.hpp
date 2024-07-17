#include <unistd.h> // For close()

class CgiPipe {
public:
	int read_fd;
	int write_fd;

	CgiPipe() : read_fd(-1), write_fd(-1) {}

	~CgiPipe() {
		closePipes();
	}

	void closePipes() {
		if (read_fd != -1) {
			close(read_fd);
			read_fd = -1; // Reset to -1 after closing
		}
		if (write_fd != -1) {
			close(write_fd);
			write_fd = -1; // Reset to -1 after closing
		}
	}
};