#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdexcept>

bool validateFile(const std::string& filePath, mode_t requiredPermissions) {
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        throw std::runtime_error("Failed to get file status");
    }

    // Check if the file owner is the same as the process owner
    if (fileStat.st_uid != getuid()) {
        std::cerr << "File owner does not match process owner" << std::endl;
        return false;
    }

    // Check if the file has the required permissions for the owner
    if ((fileStat.st_mode & requiredPermissions) != requiredPermissions) {
        std::cerr << "File does not have the required owner permissions" << std::endl;
        return false;
    }

    // Check if any group or other permissions are set
    if ((fileStat.st_mode & (S_IRWXG | S_IRWXO)) != 0) {
        std::cerr << "File has group or other permissions set" << std::endl;
        return false;
    }

    return true;
}

int main() {
    std::string filePath = "./test.sh";
    mode_t requiredPermissions = S_IRUSR | S_IWUSR; // Read and write permissions for the owner

    try {
        if (validateFile(filePath, requiredPermissions)) {
            std::cout << "File is valid" << std::endl;
        } else {
            std::cout << "File is not valid" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}

