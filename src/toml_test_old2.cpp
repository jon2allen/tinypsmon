#include <iostream>
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

class TomlReader {
public:
    TomlReader(const std::string& filename) : filename_(filename) {}

    bool read() {
        try {
            data_ = toml::parse_file(filename_);
            program_ = readProgram(data_);
            script_ = readScript(data_);
            return true;
        } catch (const toml::parse_error& e) {
            std::cerr << "Error parsing TOML file: " << e.what() << std::endl;
            return false;
        }
    }

    const Program& getProgram() const {
        return program_;
    }

    const Script& getScript() const {
        return script_;
    }

private:
    std::string filename_;
    toml::value data_;
    Program program_;
    Script script_;

    Program readProgram(const toml::value& data) {
        Program program;
        try {
            const auto& program_data = toml::find(data, "program");
            program.pgm = toml::find_or<std::string>(program_data, "pgm", "default_pgm");
            program.parms = toml::find_or<std::string>(program_data, "parms", "default_parms");
            program.user = toml::find_or<std::string>(program_data, "user", "default_user");
            program.interval_seconds = toml::find_or<int>(program_data, "interval_seconds", 0);
            program.status = toml::find_or<std::string>(program_data, "status", "unknown");
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: Missing or misspelled key in 'program' section: " << e.what() << std::endl;
        }
        return program;
    }

    Script readScript(const toml::value& data) {
        Script script;
        try {
            const auto& script_data = toml::find(data, "script");
            script.location = toml::find_or<std::string>(script_data, "location", "default_location");
            script.pgm = toml::find_or<std::string>(script_data, "pgm", "default_pgm");
            script.options = toml::find_or<std::string>(script_data, "options", "default_options");
            script.throttle_minutes = toml::find_or<int>(script_data, "throttle_minutes", 0);
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: Missing or misspelled key in 'script' section: " << e.what() << std::endl;
        }
        return script;
    }
};

int main() {
    TomlReader reader("your_toml_file.toml");
    if (reader.read()) {
        const Program& program = reader.getProgram();
        const Script& script = reader.getScript();

        std::cout << "Program:" << std::endl;
        std::cout << "  pgm: " << program.pgm << std::endl;
        std::cout << "  parms: " << program.parms << std::endl;
        std::cout << "  user: " << program.user << std::endl;
        std::cout << "  interval_seconds: " << program.interval_seconds << std::endl;
        std::cout << "  status: " << program.status << std::endl;

        std::cout << "Script:" << std::endl;
        std::cout << "  location: " << script.location << std::endl;
        std::cout << "  pgm: " << script.pgm << std::endl;
        std::cout << "  options: " << script.options << std::endl;
        std::cout << "  throttle_minutes: " << script.throttle_minutes << std::endl;
    }

    return 0;
}
