#include <fstream>
#include <iostream>
#include <string>
#include <toml.hpp>
#include <vector>

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
    int throttle_seconds;
};

class TomlParser {
public:
    TomlParser(const std::string &file_path) {
        if (!fileExists(file_path)) {
            throw std::runtime_error("TOML file not found: " + file_path);
        }
        parse(file_path);
    }

    const std::vector<Program>& getPrograms() const { return programs_; }
    const std::vector<Script>& getScripts() const { return scripts_; }

private:
    bool fileExists(const std::string &file_path) {
        std::ifstream file(file_path);
        return file.good();
    }

void parse(const std::string &file_path) {
    try {
        auto data = toml::parse(file_path);
        //  std::cout << "parse entry \n";
        int max_entries = 0;

        // Wrap the entry counting in its own try-catch
        auto program_section = toml::find(data, "program");
        try {
             for ( int i = 0; i < 10 ; i++ ) {
                    
                   std::string suffix = (i == 0) ? "" : std::to_string(i); 
                   // std::cout << "max " << suffix << "\n";
                   auto test1 = toml::find<std::string>(program_section,"pgm" + suffix);
                   // std::cout << "max " << suffix << "\n";
                    ++max_entries;
                }
         } catch (const std::out_of_range &e) {
               // std::cout << "max_entries: " << max_entries << std::endl;
                         // You can choose to handle this error or continue parsing the rest.
        }

        // Parse each program entry based on the maximum entries determined
        for (int i = 0; i < max_entries; ++i) {
            std::string suffix = (i == 0) ? "" : std::to_string(i);
            // std::cout << "pgm : " << i << "\n";
            Program program;
            program.pgm = toml::find<std::string>(program_section , "pgm" + suffix);
            program.parms = toml::find<std::string>(program_section, "parms" + suffix);
            program.user = toml::find<std::string>(program_section, "user" + suffix);
            program.interval_seconds = toml::find<int>(program_section, "interval_seconds" + suffix);
            program.status = toml::find<std::string>(program_section,  "status" + suffix);

            programs_.emplace_back(program);
        }

        auto script_section = toml::find(data, "script");
        
          // Parse each script entry based on the maximum entries determined
        for (int i = 0; i < max_entries; ++i) {
            std::string suffix = (i == 0) ? "" : std::to_string(i);
            // std::cout << "script: " << i << "\n"; 
            Script script;
            script.location = toml::find<std::string>(script_section, "location" + suffix);
            script.pgm = toml::find<std::string>(script_section, "pgm" + suffix);
            script.options = toml::find<std::string>(script_section, "options" + suffix);
            script.throttle_seconds = toml::find<int>(script_section, "throttle_seconds" + suffix);

            scripts_.emplace_back(script);
        }
    } catch (const toml::syntax_error &e) {
        throw std::runtime_error("Syntax error in TOML file: " + std::string(e.what()));
    } catch (const std::out_of_range &e) {
        throw std::runtime_error("Range error in TOML file: " + std::string(e.what()));
    } catch (const std::exception &e) {
        throw std::runtime_error("Error parsing TOML file: required key missing or mis-spelled.");
    }
}

    std::vector<Program> programs_;
    std::vector<Script> scripts_;
};

