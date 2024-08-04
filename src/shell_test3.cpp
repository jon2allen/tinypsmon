#include "shell2.hpp" // Include your class header file
#include <chrono>
#include <istream>
#include <thread>
#include <vector>

int main(int argc, char *argv[]) {

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <script_path>" << std::endl;
    return 1;
  }
  // Define the script path and arguments
  std::string scriptPath = argv[1];
  // std::string scriptPath = "./test.sh";
  std::vector<std::string> args = {"arg1", "arg2"};
  try {

    ShellScriptExecutor executor(scriptPath, args, 5);

    // Execute the script for the first time
    std::cout << "First execution: " << executor.execute() << std::endl;

    // Wait for 3 seconds and try to execute again (should be throttled)
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "Second execution (after 3 seconds): " << executor.execute()
              << std::endl;

    // Wait for another 3 seconds and try to execute again (should execute)
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "Third execution (after 6 seconds): " << executor.execute()
              << std::endl;

    return 0;
  }
  // Create an instance of ShellScriptExecutor with a throttle time of 5 seconds
  catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
