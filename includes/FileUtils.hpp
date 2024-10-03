#pragma once
#include <filesystem>
#include <string>
#include <unistd.h>

enum class FileType {
	FILE = 1,
	DIRECTORY= 2,
	OTHER = 3,
	NON_EXISTENT = -1
};

namespace FileUtils {
	namespace fs = std::filesystem;

	/**
	 * @brief Get the type of the file at the specified path.
	 * 
	 * @param path The path to the file.
	 * @return Returns the type of the file at the specified path.
	 * @retval - FileType::FILE: The file is a regular file.
	 * @retval - FileType::DIRECTORY: The file is a directory.
	 * @retval - FileType::OTHER: The file is neither a regular file nor a directory.
	 * @retval - FileType::NON_EXISTENT: The file does not exist.
	 */
	inline FileType getTypePath(const std::string& path) {
		fs::path p(path);
		try {
			if (fs::exists(p)) {
				if (fs::is_regular_file(p)) {
					return FileType::FILE;
				} else if (fs::is_directory(p)) {
					return FileType::DIRECTORY;
				} else {
					return FileType::OTHER;
				}
			} else {
				return FileType::NON_EXISTENT;
			}
		} catch(const fs::filesystem_error& e) {
			return FileType::NON_EXISTENT;
		}
	}

	/**
	* @brief Checks if the file exists and has the specified permissions.
	* 
	* @param path The path to the file.
	* @param mode The permissions to check for. This can be one or a combination of the following constants:
	* - F_OK (int:0): Test for existence.
	* - X_OK (int:1): Test for execute permission.
	* - W_OK (int:2): Test for write permission.
	* - R_OK (int:4): Test for read permission.
	* These constants can be combined using the bitwise OR operator (|). For example, to check if a file is both readable and writable, you can use R_OK | W_OK.
	* @retval 0 The file has the specified permissions.
	* @retval -1 The file does not have the specified permissions.
	*/
	inline int checkFile(const std::string& path, int mode = R_OK) {
		return access(path.c_str(), mode) == 0 ? 0 : -1;
	}


	/**
	 * @brief Checks if the file exists and is readable.
	 * 
	 * @param path The base path.
	 * @param index The index to append to the base path.
	 * @return bool Returns true if either index or path + index is a file and is readable, and false otherwise.
	 * @retval 0 The file exists and is readable.
	 * @retval -1 The file does not exist or is not readable.
	 */
	inline bool isFileExistAndReadable(const std::string& path, const std::string& index) {
		if (getTypePath(index) == FileType::FILE && checkFile(index) == 0) {
			return true;
		}
		if (getTypePath(path + index) == FileType::FILE && checkFile(path + index) == 0) {
			return true;
		}
		return false;
	}

} // namespace FileUtils