#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class ShellScriptExecutor {
public:
  ShellScriptExecutor(const std::string &scriptPath,
                      const std::vector<std::string> &args)
      : scriptPath(scriptPath), args(args) {}

  std::string execute() const {
    std::string command = scriptPath;
    for (const auto &arg : args) {
      command += " " + arg;
    }

    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"),
                                                  pclose);
    if (!pipe) {
      throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
      result += buffer.data();
    }
    return result;
  }

private:
  std::string scriptPath;
  std::vector<std::string> args;
};

int main() {
  std::vector<std::string> arguments = {"arg1", "arg2"};
  ShellScriptExecutor executor("./test.sh", arguments);
  try {
    std::string output = executor.execute();
    std::cout << "Script output:\n" << output << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}
