#pragma once
#include <sys/stat.h>
#include <unistd.h>
#include <string>

namespace FileUtils {

	/* define if path is file(1), folder(2) or something else(3) */
	inline int getTypePath(const std::string& path) {
		struct stat buffer;
		int result = stat(path.c_str(), &buffer);
		if (result == 0) {
			if (S_ISREG(buffer.st_mode)) {
				return 1;
			} else if (S_ISDIR(buffer.st_mode)) {
				return 2;
			} else {
				return 3;
			}
		} else {
			return -1;
		}
	}

	/* checks if the file exists and is accessible */
	inline int checkFile(const std::string& path, int mode) {
		return access(path.c_str(), mode);
	}

	/* checks if the file exists and is readable */
	inline int isFileExistAndReadable(const std::string& path, const std::string& index) {
		if (getTypePath(index) == 1 && checkFile(index, R_OK) == 0) {
			return 0;
		}
		if (getTypePath(path + index) == 1 && checkFile(path + index, R_OK) == 0) {
			return 0;
		}
		return -1;
	}

} // namespace FileUtils
