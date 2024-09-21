#include <iostream>
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
  int hour_of_day;        // last hour of day
  bool hourly;            // flag to handle date change to hourly

public:
  // constructor that takes the file name as a parameter and opens the file
  Logger(const std::string &fname)
      : filename(fname), file(fname, std::ios::out | std::ios::app) {
    if (!file) {
      throw std::runtime_error("Failed to open file " + fname);
    }
    // Initialize lastLogDate with the current date
    lastLogDate = getCurrentDate();

    hourly = false;   // set true for test
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
  // internal log - no mutex no date change
  void ilog(const std::string &message) {
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

  // Public method to allow testing of deleteOldFiles
  void testDeleteOldFiles(const std::string &directory, int daysOld) {
    deleteOldFiles(directory, daysOld);
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
  //////
  // New method to get the current hour of the day
  // for testing.
  //////////////////////
  int getHourOfDay() {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm localTime = *std::localtime(&tt); // Convert to local time

    return localTime.tm_hour; // Return the hour (0-23)
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
    std::string prefix = filename; // Get the filename prefix
    std::cout << "logger internal: ... starting delete older files older than " << daysOld << " days old" << std::endl;
    std::cout << "logger internal: delete file prefix - " << prefix << std::endl;
    ilog("logger internal:  ... starting delete older files older than " + std::to_string(daysOld) + " days old");
    ilog("logger internal:  delete file prefix - " + prefix );
    for (const auto &entry : std::filesystem::directory_iterator(directory)) {
      if (entry.is_regular_file()) { // Ensure it's a regular file
        auto fileName = entry.path().filename().string();
        // log("logger internal : file prefix match - " + entry.path().filename().string() );
        // Check if the file starts with the prefix 
        if (fileName.rfind(prefix, 0) == 0 ) {
          auto fileTime = std::filesystem::last_write_time(entry);
          auto sctp =
              std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                  fileTime - std::filesystem::file_time_type::clock::now() +
                  std::chrono::system_clock::now());
          auto fileAge =
              std::chrono::duration_cast<std::chrono::hours>(now - sctp).count() /
              24;
          std::cout << "logger internal: prefix match - " << entry.path().filename().string() << std::endl;
          ilog("logger internal: prefix match - " + entry.path().filename().string()); 
          if (fileAge > daysOld) {
            ilog("logger internal: removing - " + entry.path().string());
            std::cout << "removing: " << entry << std::endl;
            std::filesystem::remove(entry);
          }
        }
      }
    }
  }

// New function to handle file renaming, gzipping, and opening a new log file
void zip_and_rotate() {
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

    // Delete old files
    deleteOldFiles(".", 10);
}



void handleDateChange() {

    // Get the current date
    std::string currentDate = getCurrentDate();
    int currentHour = getHourOfDay();
    if (hourly == true){
         if (hour_of_day != currentHour ) {
             zip_and_rotate();
         }
         hour_of_day = currentHour;
         return;
    } 
    // Check if the date has changed
    if (currentDate != lastLogDate) {
        zip_and_rotate();  // Call the new zip_and_rotate function
        lastLogDate = currentDate;  // Update the last log date
    }
    hour_of_day = currentHour;
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

