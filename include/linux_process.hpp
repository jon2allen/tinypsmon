#include <dirent.h>

struct ProcessInfo {
   std::string name;
   int pid;
   std::string user; // Add a field for the username
   std::vector<std::string> arguments;
};

 struct matchProcess {
   std::string process_name;
   std::string username;
   std::string argument;
};

class ProcessLister {
public:

   std::vector<ProcessInfo> getProcesses() {
        std::vector<ProcessInfo> processList;
        std::string line;
        std::ifstream procFile("/proc");

        for (const auto &entry : std::filesystem::directory_iterator("/proc")) {
            if (entry.is_directory()) {
                std::string pidStr = entry.path().filename().string();
                if (std::all_of(pidStr.begin(), pidStr.end(), ::isdigit)) {
                    ProcessInfo proc;
                    proc.pid = std::stoi(pidStr);

                    std::ifstream cmdlineFile(entry.path() / "cmdline");
                    std::getline(cmdlineFile, line, '\0');
                    std::istringstream iss(line);
                    std::getline(iss, proc.name, '\0');

                    std::ifstream statusFile(entry.path() / "status");
                    while (std::getline(statusFile, line)) {
                        if (line.find("Uid:") == 0) {
                            std::istringstream uidStream(line.substr(5));
                            int uid;
                            uidStream >> uid;
                            struct passwd *pw = getpwuid(uid);
                            proc.user = (pw != nullptr) ? pw->pw_name : "Unknown";
                            break;
                        }
                    }

                    std::ifstream argsFile(entry.path() / "cmdline");
                    while (std::getline(argsFile, line, '\0')) {
                        proc.arguments.push_back(line);
                    }

                    processList.push_back(proc);
                }
            }
        }
        return processList;
    }


  void printProcesses(const std::vector<ProcessInfo> &processList) {
    for (const auto &proc : processList) {
      std::cout << "Process ID: " << proc.pid
                << ", Process Name: " << proc.name;
      std::cout << ", User: " << proc.user; // Print the username
      if (!proc.arguments.empty()) {
        std::cout << ", Arguments: ";
        for (const auto &arg : proc.arguments) {
          std::cout << arg << ' ';
        }
      }
      std::cout << "num args: " << proc.arguments.size() << " ";
      std::cout << std::endl;
    }
  }

  void logSingleProcess(const ProcessInfo &proc) {
    std::string logMessage = "Process ID: " + std::to_string(proc.pid) +
                             ", Process Name: " + proc.name +
                             ", User: " + proc.user;

    if (!proc.arguments.empty()) {
      logMessage += ", Arguments: ";
      for (const auto &arg : proc.arguments) {
        logMessage += arg + ' ';
      }
    }

    logMessage += "num args: " + std::to_string(proc.arguments.size()) + " ";
    logger.log(logMessage);
  }

  void logProcesses(const std::vector<ProcessInfo> &processList) {
    for (const auto &proc : processList) {
      logSingleProcess(proc);
    }
  }

  bool searchProcess(const std::vector<ProcessInfo> &processList,
                     const matchProcess &searchCriteria,
                     const ProcessInfo *&foundProcess) {
    foundProcess = nullptr;
    // Iterate through the list of processes
    for (const auto &process : processList) {
      // Check if the process name matches
      if (process.name == searchCriteria.process_name) {
        // Check if the username matches
        // logger.log(" match - process name: " + searchCriteria.process_name);
        if (process.user == searchCriteria.username) {
          // Check if any argument matches
          // logger.log(" match - user name: " + searchCriteria.username);
          if (std::any_of(process.arguments.begin(), process.arguments.end(),
                          [&](const std::string &arg) {
                            return arg.find(searchCriteria.argument) !=
                                   std::string::npos;
                          })) {
            logger.log(" matach -> " + searchCriteria.argument);
            logger.log(" match - user name: " + searchCriteria.username);
            logger.log(" match - process name: " + searchCriteria.process_name);
            foundProcess = &process;
            return true; // All conditions met
          }
        }
      }
    }
    logger.log(" no match found -> process: " + searchCriteria.process_name +
               " user:  " + searchCriteria.username);
    return false; // No matching process found
  }
};

