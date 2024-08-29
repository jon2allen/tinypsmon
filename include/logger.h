#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <zlib.h>

class Logger {
  std::string filename;    // the name of the file to write to
  std::ofstream file;      // the file stream object
  std::string lastLogDate; // the date of the last log entry
  std::mutex log_mtx;

public:
  // constructor that takes the file name as a parameter and opens the file
  Logger(const std::string &fname)
      : filename(fname), file(fname, std::ios::out | std::ios::app) {
    if (!file) {
      throw std::runtime_error("Failed to open file " + fname);
    }
    // Initialize lastLogDate with the current date
    lastLogDate = getCurrentDate();
  }

  // destructor that closes the file
  ~Logger() { file.close(); }

  // method that takes a message as a parameter and writes it to the file with a
  // timestamp
  void log(const std::string &message) {
    std::lock_guard<std::mutex> lock(log_mtx);
    handleDateChange();
    logMessageWithTimestamp(message);
  }

  // New method to handle multiline input and log each line with a timestamp
  // prefix
  void logMultiline(const std::string &multilineInput) {
    std::lock_guard<std::mutex> lock(log_mtx);
    handleDateChange();
    std::istringstream iss(multilineInput);
    std::string line;
    while (std::getline(iss, line)) {
      logMessageWithTimestamp(line);
    }
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

  void gzipFile(const std::string &filename) {
    std::string gzFilename = filename + ".gz";
    gzFile gzFile = gzopen(gzFilename.c_str(), "wb");
    if (!gzFile) {
      throw std::runtime_error("Failed to open gzip file " + gzFilename);
    }

    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
      gzclose(gzFile);
      throw std::runtime_error("Failed to open file " + filename);
    }

    char buffer[1024];
    while (inFile.read(buffer, sizeof(buffer))) {
      gzwrite(gzFile, buffer, inFile.gcount());
    }
    gzwrite(gzFile, buffer, inFile.gcount()); // Write any remaining data

    inFile.close();
    gzclose(gzFile);

    // Remove the original file after gzipping
    std::filesystem::remove(filename);
  }

  // Function to delete files older than a specified number of days

  void deleteOldFiles(const std::string &directory, int daysOld) {
    auto now = std::chrono::system_clock::now();
    for (const auto &entry : std::filesystem::directory_iterator(directory)) {
      auto fileTime = std::filesystem::last_write_time(entry);
      auto sctp =
          std::chrono::time_point_cast<std::chrono::system_clock::duration>(
              fileTime - std::filesystem::file_time_type::clock::now() +
              std::chrono::system_clock::now());
      auto fileAge =
          std::chrono::duration_cast<std::chrono::hours>(now - sctp).count() /
          24;
      if (fileAge > daysOld) {
        std::filesystem::remove(entry);
      }
    }
  }

  // Helper method to handle date change
  void handleDateChange() {
    // Get the current date
    std::string currentDate = getCurrentDate();

    // Check if the date has changed
    if (currentDate != lastLogDate) {
      // Close the current file
      file.close();

      // Rename the file to include the date
      std::string newFilename = filename + "." + lastLogDate;
      std::filesystem::rename(filename, newFilename);

      // Gzip the renamed file
      gzipFile(newFilename);

      // Open a new file for logging
      file.open(filename, std::ios::out | std::ios::app);
      if (!file) {
        throw std::runtime_error("Failed to open file " + filename);
      }

      // Update the last log date
      lastLogDate = currentDate;
    }
  }
  // Helper method to log a message with a timestamp
  void logMessageWithTimestamp(const std::string &message) {
    // Get the current time
    auto time = std::chrono::system_clock::now();
    time_t tt = std::chrono::system_clock::to_time_t(time);
    std::tm tm = *std::localtime(&tt);

    // Write the formatted time and date and the message to the file, separated
    // by a space
    file << std::put_time(&tm, "%D %r %Z") << " " << message << "\n";
    file.flush();
  }
};
