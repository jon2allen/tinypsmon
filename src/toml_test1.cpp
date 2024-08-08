#include <iostream>
#include <string>
#include <toml.hpp>

struct Program {
    std::string pgm;
    std::string parms;
    std::string user;
    int interval_seconds;
    std::string status;
};

struct Script {
    std::string location;
    std::string pgm;
    std::string options;
    int throttle_minutes;
};

class ConfigReader {
public:
    ConfigReader(const std::string& file_path) {
        data = toml::parse(file_path);
        parseProgram();
        parseScript();
    }

    Program getProgram() const {
        return program;
    }

    Script getScript() const {
        return script;
    }

private:
    toml::value data;
    Program program;
    Script script;

    void parseProgram() {
        const auto& prog = toml::find(data, "program");
        program.pgm = toml::find<std::string>(prog, "pgm");
        program.parms = toml::find<std::string>(prog, "parms");
        program.user = toml::find<std::string>(prog, "user");
        program.interval_seconds = toml::find<int>(prog, "interval_seconds");
        program.status = toml::find<std::string>(prog, "status");
    }

    void parseScript() {
        const auto& scr = toml::find(data, "script");
        script.location = toml::find<std::string>(scr, "location");
        script.pgm = toml::find<std::string>(scr, "pgm");
        script.options = toml::find<std::string>(scr, "options");
        script.throttle_minutes = toml::find<int>(scr, "throttle_minutes");
    }
};

int main() {
    ConfigReader reader("test.toml");

    Program prog = reader.getProgram();
    std::cout << "Program:\n";
    std::cout << "pgm: " << prog.pgm << "\n";
    std::cout << "parms: " << prog.parms << "\n";
    std::cout << "user: " << prog.user << "\n";
    std::cout << "interval_seconds: " << prog.interval_seconds << "\n";
    std::cout << "status: " << prog.status << "\n";

    Script scr = reader.getScript();
    std::cout << "Script:\n";
    std::cout << "location: " << scr.location << "\n";
    std::cout << "pgm: " << scr.pgm << "\n";
    std::cout << "options: " << scr.options << "\n";
    std::cout << "throttle_minutes: " << scr.throttle_minutes << "\n";

    return 0;
}

