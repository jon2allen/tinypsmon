#include <fstream>
#include <iostream>
#include <string>
#include <toml.hpp>

struct Program {
  std::string pgm;
  std::string parms;
  std::string user;
  int interval_minutes;
  std::string status;
};

struct Script {
  std::string location;
  std::string pgm;
  std::string options;
  int throttle_minutes;
};

class TomlParser {
public:
  TomlParser(const std::string &file_path) {
    if (!fileExists(file_path)) {
      throw std::runtime_error("TOML file not found: " + file_path);
    }
    parse(file_path);
  }

  const Program &getProgram() const { return program_; }

  const Script &getScript() const { return script_; }

private:
  bool fileExists(const std::string &file_path) {
    std::ifstream file(file_path);
    return file.good();
  }

  void parse(const std::string &file_path) {
    try {
      auto data = toml::parse(file_path);

      // Parse program section
      auto program = toml::find(data, "program");
      program_.pgm = toml::find<std::string>(program, "pgm");
      program_.parms = toml::find<std::string>(program, "parms");
      program_.user = toml::find<std::string>(program, "user");
      program_.interval_minutes = toml::find<int>(program, "interval_minutes");
      program_.status = toml::find<std::string>(program, "status");

      // Parse script section
      auto script = toml::find(data, "script");
      script_.location = toml::find<std::string>(script, "location");
      script_.pgm = toml::find<std::string>(script, "pgm");
      script_.options = toml::find<std::string>(script, "options");
      script_.throttle_minutes = toml::find<int>(script, "throttle_minutes");
    } catch (const toml::syntax_error &e) {
      throw std::runtime_error("Syntax error in TOML file: " +
                               std::string(e.what()));
    } catch (const toml::type_error &e) {
      throw std::runtime_error("Type error in TOML file: " +
                               std::string(e.what()));
    } catch (const std::exception &e) {
      throw std::runtime_error("Error parsing TOML file: " +
                               std::string(e.what()));
    }
  }

  Program program_;
  Script script_;
};

int main() {
  try {
    std::cout << "Toml test pgm \n";
    TomlParser parser("my_key.toml");

    const Program &program = parser.getProgram();
    std::cout << "Program:\n";
    std::cout << "  pgm: " << program.pgm << "\n";
    std::cout << "  parms: " << program.parms << "\n";
    std::cout << "  user: " << program.user << "\n";
    std::cout << "  interval_minutes: " << program.interval_minutes << "\n";
    std::cout << "  status: " << program.status << "\n";

    const Script &script = parser.getScript();
    std::cout << "Script:\n";
    std::cout << "  location: " << script.location << "\n";
    std::cout << "  pgm: " << script.pgm << "\n";
    std::cout << "  options: " << script.options << "\n";
    std::cout << "  throttle_minutes: " << script.throttle_minutes << "\n";
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
