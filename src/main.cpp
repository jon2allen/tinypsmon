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
  Program program;
  Script script_info;
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
    // if (_found == false) {
    //   ps.logProcesses(processes);
    //  }
    //  ** maybe have some verbose debug option that dumps
    _found = ps.searchProcess(processes, _m, ps_t);
    if (_found == true) {

      logger.log("process:  " + _m.process_name + " found");
    }
    if (ps_status == _found) {
      // ps.logProcesses(processes);
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
  std::optional<InitializationResult> _parms;
  bool _found = false;
  ShellScriptExecutor _shell_1;
  bool ps_status = false;
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
//*********************************************
// process statue
//  ps target is when script is fired
//  so if target id down - process now running
//  then execute script
//***********************************************
bool processState(std::string status) {
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

// ----------------------------------------------------------------------------
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

void processCmdLine(std::string cmdparm) {
  // ps is global
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
  std::vector<std::string> opts = {initResult->script_info.options};
  std::string script1 =
      initResult->script_info.location + "/" + initResult->script_info.pgm;
  auto alarm_sh = ShellScriptExecutor(script1, opts,
                                      initResult->script_info.throttle_seconds);

  if (alarm_sh.isShellgood() == false) {
    std::cout << "bad script: " << script1 << "  - correct toml config "
              << std::endl;
    exit(4);
  }
  const struct ::timespec rqt = {100, 0};

  struct ::matchProcess m = {initResult->program.pgm, initResult->program.user,
                             initResult->program.parms};

  bool ps_state = processState(initResult->program.status);

  mypoll pspoll("mypoll", m, alarm_sh, ps_state);
  TimerAlarm<mypoll> timer(pspoll, initResult->program.interval_seconds);

  timer.arm();

  printBanner();
  while (true) {
    logger.log("Main loop");
    std::vector<ProcessInfo> processes = ps.getProcesses();
    ps.logProcesses(processes);
    // std::cout << "sleeping...  \n";
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
