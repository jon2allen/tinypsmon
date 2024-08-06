#include <chrono>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <filesystem>

class Logger {
    std::string filename; // the name of the file to write to
    std::ofstream file; // the file stream object
    std::string lastLogDate; // the date of the last log entry

public:
    // constructor that takes the file name as a parameter and opens the file
    Logger(const std::string& fname) : filename(fname), file(fname, std::ios::out | std::ios::app) {
        if (!file) {
            throw std::runtime_error("Failed to open file " + fname);
        }
        // Initialize lastLogDate with the current date
        lastLogDate = getCurrentDate();
    }

    // destructor that closes the file
    ~Logger() {
        file.close();
    }

    // method that takes a message as a parameter and writes it to the file with a timestamp
    void log(const std::string& message) {
        // Get the current date
        std::string currentDate = getCurrentDate();

        // Check if the date has changed
        if (currentDate != lastLogDate) {
            // Close the current file
            file.close();

            // Rename the file to include the date
            std::string newFilename = filename + "." + lastLogDate;
            std::filesystem::rename(filename, newFilename);

            // Open a new file for logging
            file.open(filename, std::ios::out | std::ios::app);
            if (!file) {
                throw std::runtime_error("Failed to open file " + filename);
            }

            // Update the last log date
            lastLogDate = currentDate;
        }

        // Get the current time
        auto time = std::chrono::system_clock::now();
        time_t tt = std::chrono::system_clock::to_time_t(time);
        std::tm tm = *std::localtime(&tt);

        // Write the formatted time and date and the message to the file, separated by a space
        file << std::put_time(&tm, "%D %r %Z") << " " << message << "\n";
        file.flush();
    }

private:
    // Helper method to get the current date as a string
    std::string getCurrentDate() {
        auto time = std::chrono::system_clock::now();
        time_t tt = std::chrono::system_clock::to_time_t(time);
        std::tm tm = *std::localtime(&tt);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d");
        return oss.str();
    }
};

