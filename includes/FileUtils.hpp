#pragma once
#include <filesystem>
#include <string>
#include <unistd.h>

namespace FileUtils {
	namespace fs = std::filesystem;

	/* define if path is file(1), folder(2) or something else(3) */
	inline int getTypePath(const std::string& path) {
		fs::path p(path);
		try {
			if (fs::exists(p)) {
				if (fs::is_regular_file(p)) {
					return 1;
				} else if (fs::is_directory(p)) {
					return 2;
				} else {
					return 3;
				}
			} else {
				return -1;
			}
		} catch(const fs::filesystem_error& e) {
			return -1;
		}
	}

	/**
	* @brief Checks if the file exists and has the specified permissions.
	* 
	* @param path The path to the file.
	* @param mode The permissions to check for. This can be one or a combination of the following constants:
	* - R_OK: Test for read permission.
	* - W_OK: Test for write permission.
	* - X_OK: Test for execute (or search) permission.
	* - F_OK: Test for existence of the file.
	* These constants can be combined using the bitwise OR operator (|). For example, to check if a file is both readable and writable, you can use R_OK | W_OK.
	* @return int Returns 0 if the file has the specified permissions, and -1 otherwise.
	*/
	inline int checkFile(const std::string& path, int mode = R_OK) {
		return access(path.c_str(), mode) == 0 ? 0 : -1;
	}


	/**
	 * @brief Checks if the file exists and is readable.
	 * 
	 * @param path The base path.
	 * @param index The index to append to the base path.
	 * @return int Returns 0 if either index or path + index is a file and is readable, and -1 otherwise.
	 */
	inline int isFileExistAndReadable(const std::string& path, const std::string& index) {
		if (getTypePath(index) == 1 && checkFile(index) == 0) {
			return 0;
		}
		if (getTypePath(path + index) == 1 && checkFile(path + index) == 0) {
			return 0;
		}
		return -1;
	}

} // namespace FileUtils
