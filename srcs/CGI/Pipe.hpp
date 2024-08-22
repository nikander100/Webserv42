#pragma once
#include <unistd.h>

class Pipe {
	public:
		// Public references for easy access

		// fd[0]
		int& read_fd = pipefds[0];
		// fd[1]
		int& write_fd = pipefds[1];

		Pipe() : read_fd(pipefds[0]), write_fd(pipefds[1]) {
			pipefds[0] = -1; // Initialize to -1
			pipefds[1] = -1; // Initialize to -1
		}

		~Pipe() {
			closePipe();
		}

		// Closes both fd[0] and fd[1] of the pipe
		void closePipe() {
			if (pipefds[0] != -1) {
				closeRead();
			}
			if (pipefds[1] != -1) {
				closeWrite();
			}
		}

		// Closes fd[0] to read from the pipe
		void closeRead() {
			if (pipefds[0] != -1) {
				close(pipefds[0]);
				pipefds[0] = -1; // Reset to -1 after closing
			}
		}

		// Closes fd[1] to write to the pipe
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