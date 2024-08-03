// jona test timer

#include "TimerAlarm.h"
#include "logger.h"
#include "shell.hpp"
#include "toml_reader.hpp"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <kvm.h>
#include <optional>
#include <paths.h>
#include <pwd.h> // Include for getpwuid
#include <stdint.h>
#include <string>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/user.h>
#include <vector>

Logger logger("new_log.txt");

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

  bool searchProcess(const std::vector<ProcessInfo> &processList,
                     const matchProcess &searchCriteria) {
    // Iterate through the list of processes
    for (const auto &process : processList) {
      // Check if the process name matches
      if (process.name == searchCriteria.process_name) {
        // Check if the username matches
        logger.log(" match - process name: " + searchCriteria.process_name);
        if (process.user == searchCriteria.username) {
          // Check if any argument matches
          logger.log(" match - user name: " + searchCriteria.username);
          if (std::any_of(process.arguments.begin(), process.arguments.end(),
                          [&](const std::string &arg) {
                            return arg.find(searchCriteria.argument) !=
                                   std::string::npos;
                          })) {
            logger.log(" matach -> " + searchCriteria.argument);
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

using namespace hmta;

// ----------------------------------------------------------------------------

struct InitializationResult {
  Program program;
  Script script_info;
};

ProcessLister ps;

class mypoll {

public:
  mypoll(std::string s, matchProcess &m, ShellScriptExecutor my_shell)
      : _s(s), _m(m), _shell_1(my_shell) {
    std::vector<std::string> opts = {"arg1"};

    _shell_1 = ShellScriptExecutor("./test.sh", opts);
  }

  bool operator()() {

    std::cout << "mypoll:  " << _s << std::endl;
    logger.log("Testing ps... ");
    std::vector<ProcessInfo> processes = ps.getProcesses();
    ps.printProcesses(processes);
    _found = ps.searchProcess(processes, _m);
    if (_found == true) {
      std::cout << "process:  " << _m.process_name << " found \n";
    } else {
      std::cout << "not found \n";
      std::string output = _shell_1.execute();
      std::cout << "Script output: \n" << output << std::endl;
    }
    return (true);
  }

private:
  std::string _s;
  matchProcess &_m;
  std::optional<InitializationResult> _parms;
  bool _found = false;
  ShellScriptExecutor _shell_1;
};

std::optional<InitializationResult> initialize(const std::string &file_path) {
  try {
    TomlParser parser(file_path);
    const Program &ps1 = parser.getProgram();
    const Script script1 = parser.getScript();
    std::cout << "  pgm: " << ps1.pgm << "\n";
    std::cout << "  parms: " << ps1.parms << "\n";
    std::cout << "  user: " << ps1.user << "\n";

    return InitializationResult{ps1, script1};
  } catch (const std::exception &e) {
    std::cerr << "Invalid TOML file: " << e.what() << std::endl;
    return std::nullopt;
  }
}

// ----------------------------------------------------------------------------

int main(int, char *[]) {
  auto initResult = initialize("config.toml");
  if (!initResult) {
    return EXIT_FAILURE;
  }

  std::vector<std::string> opts = {"arg1"};

  auto alarm_sh = ShellScriptExecutor("./test.sh", opts);

  const struct ::timespec rqt = {100, 0};

  struct ::matchProcess m = {initResult->program.pgm, initResult->program.user,
                             initResult->program.parms};

  mypoll pspoll("mypoll", m, alarm_sh);
  TimerAlarm<mypoll> timer(pspoll, initResult->program.interval_seconds);

  timer.arm();
  while (true) {
    std::cout << "starting...  \n";

    std::cout << "sleeping...  \n";
    nanosleep(&rqt, 0);
  }
  return (EXIT_SUCCESS);
}

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
