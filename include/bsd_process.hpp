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
  std::set<int> pid_cache;

  std::vector<ProcessInfo> getProcesses() {
    std::vector<ProcessInfo> processList;
    kvm_t *kd = kvm_open(NULL, _PATH_DEVNULL, NULL, O_RDONLY, "kvm_open");
    if (kd != nullptr) {
      int count;
      struct kinfo_proc *procs = kvm_getprocs(kd, KERN_PROC_PROC, 0, &count);
      if (procs == nullptr) {
        std::cerr << "Failed to get processes" << std::endl;
        kvm_close(kd);
        return processList;
      }

      for (int i = 0; i < count; i++) {
        ProcessInfo proc;
        proc.pid = procs[i].ki_pid;
        proc.name = procs[i].ki_comm;

        // Get the username from the UID
        struct passwd *pw = getpwuid(procs[i].ki_uid);
        if (pw != nullptr) {
          proc.user = pw->pw_name;
        } else {
          proc.user = "Unknown";
        }

        // Get the command line arguments for the process
        char **argv = kvm_getargv(kd, &procs[i], 0);
        if (argv != nullptr) {
          while (*argv) {
            proc.arguments.push_back(*argv);
            argv++;
          }
        }

        processList.push_back(proc);
        if (searchPidcache(proc.pid) == true) {
          logger.log("found pid: : " + std::to_string(proc.pid));
          std::swap(processList.front(), processList.back());
        }
      }
      kvm_close(kd);
    } else {
      std::cerr << "Unable to open kvm" << std::endl;
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

  void setPidcache(const int p) {
    if ( pid_cache.find(p) == pid_cache.end()) {
        pid_cache.insert(p); 
        logger.log( "pid cache:  " + std::to_string( pid_cache.size() ) );
     }
  }

  bool searchPidcache(const int p) {
    for (const auto &mypid : pid_cache) {
      if (mypid == p) {
        return true;
      }
    }
    return false;
  }

  void flushPidcache() { 
     if ( pid_cache.size() > 10 ) {
        pid_cache.clear();
        logger.log("flushing cache");
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
            setPidcache(process.pid);
            return true; // All conditions met
          }
        }
      }
    }
    logger.log(" no match found -> process: " + searchCriteria.process_name +
               " user:  " + searchCriteria.username);
    flushPidcache();
    return false; // No matching process found
  }
};
