// *******************************
// Programe:  Tinypsmon
// Creator:  Jon Allen
// License:  BSD
// ******************************

#include "TimerAlarm.h"
#include "logger.h"
#include "shell.hpp"
#include "toml_reader.hpp"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <ios>
#include <iostream>
#ifdef __FreeBSD__
#include <kvm.h>  //freebsd
#endif
#include <optional>
#include <paths.h>
#include <pwd.h> // Include for getpwuid
#include <stdint.h>
#include <string>
#include  <set>
#ifdef __FreeBSD__
#include <sys/sysctl.h>  // freebsd
#endif
#include <sys/types.h>
#include <sys/user.h>
#include <vector>
Logger logger("tinypsmon.log");
#ifdef __FreeBSD__
#include "bsd_process.hpp"
#endif
#ifndef __FreeBSD__
#include "linux_process.hpp"
#endif

using namespace hmta;

// ----------------------------------------------------------------------------

struct InitializationResult {
  std::vector<Program> programs;
  std::vector<Script> scripts;
};

ProcessLister ps;

class mypoll {

public:
  mypoll(std::string s, matchProcess &m, ShellScriptExecutor my_shell,
         bool ps_s)
      : _s(s), _m(m), _shell_1(my_shell), ps_status(ps_s) {}

  bool operator()() {
    const ProcessInfo *ps_t = nullptr;
    std::cout << "mypoll:  " << _s << std::endl;
    logger.log("Testing ps... ");
    std::vector<ProcessInfo> processes = ps.getProcesses();
    
    _found = ps.searchProcess(processes, _m, ps_t);
    if (_found == true) {
      logger.log("process:  " + _m.process_name + " found");
    }
    
    if (ps_status == _found) {
      if (ps_t != nullptr) {
        ps.logSingleProcess(*ps_t);
      }
      std::cout << "status change.. running script\n";
      logger.log("status change.. running script");
      std::string output = _shell_1.execute();
      std::cout << "Script output: \n" << output << std::endl;
      logger.log("Script output start: ");
      logger.logMultiline(output);
      logger.log("Script output end:");
    }
    return (true);
  }

private:
  std::string _s;
  matchProcess &_m;
  bool _found = false;
  ShellScriptExecutor _shell_1;
  bool ps_status = false;
};

std::optional<InitializationResult> initialize(const std::string &file_path) {
  try {
    TomlParser parser(file_path);
    const auto &programs = parser.getPrograms();
    const auto &scripts = parser.getScripts();

    // Ensure we have at least one program and one script
    if (programs.empty() || scripts.empty()) {
      std::cerr << "No valid program or script entries found in the TOML file." << std::endl;
      return std::nullopt;
    }

    // Log first program and script for initialization confirmation
    std::cout << "  pgm: " << programs[0].pgm << "\n";
    std::cout << "  parms: " << programs[0].parms << "\n";
    std::cout << "  user: " << programs[0].user << "\n";

    return InitializationResult{programs, scripts};
  } catch (const std::exception &e) {
    std::cerr << "Invalid TOML file: " << e.what() << std::endl;
    return std::nullopt;
  }
}

bool processState(const std::string &status) {
  if (status == "down") {
    logger.log(" desired ps target is down");
    return false;
  }
  if (status == "up") {
    logger.log(" desired ps target is up ");
    return true;
  } else {
    logger.log(" Unknown ps target " + status + " setting to default down");
    return false;
  }
}

void printBanner() {
  const int width = 40;
  std::string programName = "tinypsmon";
  std::string compileDate = __DATE__;
  std::string author = "Jon Allen";

  // Print top border
  std::cout << std::string(width, '*') << std::endl;

  // Print program name
  std::cout << "*" << std::left << std::setw(width - 2)
            << (" Program Name: " + programName) << "*" << std::endl;
  logger.log("Starting Program Name: " + programName);

  // Print compile date
  std::cout << "*" << std::left << std::setw(width - 2)
            << (" Compiled on: " + compileDate) << "*" << std::endl;
  logger.log(" Compiled on: " + compileDate);

  // Print author
  std::cout << "*" << std::left << std::setw(width - 2)
            << (" Author: " + author) << "*" << std::endl;
  logger.log(" Author: " + author);

  // Print bottom border
  std::cout << std::string(width, '*') << std::endl;
}

void processCmdLine(const std::string &cmdparm) {
  if (cmdparm == "-pslist" || cmdparm == "--ps") {
    std::vector<ProcessInfo> processes = ps.getProcesses();
    ps.printProcesses(processes);
  } else {
    std::cerr << "Cmdline invalid:  " << cmdparm << std::endl;
  }
}

int main(int argc, char *argv[]) {
  auto initResult = initialize("config.toml");
  if (!initResult) {
    return EXIT_FAILURE;
  }

  if (argc > 1) {
    std::string cmdline1 = argv[1];
    processCmdLine(cmdline1);
    exit(0);
  }

  // Use the first program and script for simplicity
  const Program &program = initResult->programs[0];
  const Script &script = initResult->scripts[0];

  std::vector<std::string> opts = {script.options};
  std::string script1 = script.location + "/" + script.pgm;
  auto alarm_sh = ShellScriptExecutor(script1, opts, script.throttle_seconds);

  if (!alarm_sh.isShellgood()) {
    std::cout << "bad script: " << script1 << "  - correct toml config "
              << std::endl;
    exit(4);
  }

  const struct ::timespec rqt = {100, 0};

  matchProcess m = {program.pgm, program.user, program.parms};
  bool ps_state = processState(program.status);

  mypoll pspoll("mypoll", m, alarm_sh, ps_state);
  TimerAlarm<mypoll> timer(pspoll, program.interval_seconds);

  timer.arm();
  printBanner();

  while (true) {
    logger.log("Main loop");
    std::vector<ProcessInfo> processes = ps.getProcesses();
    ps.logProcesses(processes);
    nanosleep(&rqt, nullptr);
  }

  return EXIT_SUCCESS;
}

