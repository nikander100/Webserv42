#include <unistd.h>

class Pipe {
	public:
		// Public references for easy access
		int& read_fd = pipefds[0];
		int& write_fd = pipefds[1];

		Pipe() : read_fd(pipefds[0]), write_fd(pipefds[1]) {
			pipefds[0] = -1; // Initialize to -1
			pipefds[1] = -1; // Initialize to -1
		}

		~Pipe() {
			closePipe();
		}

		void closePipe() {
			if (pipefds[0] != -1) {
				close(pipefds[0]);
				pipefds[0] = -1; // Reset to -1 after closing
			}
			if (pipefds[1] != -1) {
				close(pipefds[1]);
				pipefds[1] = -1; // Reset to -1 after closing
			}
		}

		void closeRead() {
			if (pipefds[0] != -1) {
				close(pipefds[0]);
				pipefds[0] = -1; // Reset to -1 after closing
			}
		}

		void closeWrite() {
			if (pipefds[1] != -1) {
				close(pipefds[1]);
				pipefds[1] = -1; // Reset to -1 after closing
			}
		}

		bool createPipe() {
			if (pipe(pipefds) < 0) {
				return false;
			}
			return true;
		}

	private:
		int pipefds[2]; // Array to hold the pipe file descriptors, now public
};