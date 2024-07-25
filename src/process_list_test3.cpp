#include <stdint.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#include <kvm.h>
#include <paths.h>
#include <fcntl.h>
#include <vector>
#include <string>
#include <iostream>
#include <pwd.h> // Include for getpwuid

struct ProcessInfo {
    std::string name;
    int pid;
    std::string user; // Add a field for the username
    std::vector<std::string> arguments;
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

    void printProcesses(const std::vector<ProcessInfo>& processList) {
        for (const auto& proc : processList) {
            std::cout << "Process ID: " << proc.pid << ", Process Name: " << proc.name;
            std::cout << ", User: " << proc.user; // Print the username
            if (!proc.arguments.empty()) {
                std::cout << ", Arguments: ";
                for (const auto& arg : proc.arguments) {
                    std::cout << arg << ' ';
                }
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    ProcessLister lister;
    std::vector<ProcessInfo> processes = lister.getProcesses();
    lister.printProcesses(processes);
    return 0;
}

