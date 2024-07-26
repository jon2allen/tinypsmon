#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <array>

class ShellScriptExecutor {
public:
    ShellScriptExecutor(const std::string& scriptPath, const std::vector<std::string>& args)
        : scriptPath(scriptPath), args(args) {
        validateInputs();
    }

    std::string execute() const {
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            throw std::runtime_error("pipe() failed");
        }

        pid_t pid = fork();
        if (pid == -1) {
            throw std::runtime_error("fork() failed");
        }

        if (pid == 0) { // Child process
            close(pipefd[0]); // Close read end
            dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
            close(pipefd[1]);

            std::vector<char*> execArgs;
            execArgs.push_back(const_cast<char*>(scriptPath.c_str()));
            for (const auto& arg : args) {
                execArgs.push_back(const_cast<char*>(arg.c_str()));
            }
            execArgs.push_back(nullptr);

            execvp(scriptPath.c_str(), execArgs.data());
            _exit(EXIT_FAILURE); // execvp failed
        } else { // Parent process
            close(pipefd[1]); // Close write end

            std::array<char, 128> buffer;
            std::string result;
            ssize_t count;
            while ((count = read(pipefd[0], buffer.data(), buffer.size())) > 0) {
                result.append(buffer.data(), count);
            }
            close(pipefd[0]);

            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                throw std::runtime_error("Script execution failed with status " + std::to_string(WEXITSTATUS(status)));
            }

            return result;
        }
    }

private:
    std::string scriptPath;
    std::vector<std::string> args;

    void validateInputs() const {
        if (scriptPath.empty()) {
            throw std::invalid_argument("Script path cannot be empty");
        }
        // Add more validation as needed
    }
};

int main() {
    std::vector<std::string> arguments = {"arg1", "arg2"};
    ShellScriptExecutor executor("./test.sh", arguments);
    try {
        std::string output = executor.execute();
        std::cout << "Script output:\n" << output << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}

