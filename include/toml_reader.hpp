#include <fstream>
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
      program_.interval_seconds = toml::find<int>(program, "interval_seconds");
      program_.status = toml::find<std::string>(program, "status");

      // Parse script section
      auto script = toml::find(data, "script");
      script_.location = toml::find<std::string>(script, "location");
      script_.pgm = toml::find<std::string>(script, "pgm");
      script_.options = toml::find<std::string>(script, "options");
      script_.throttle_seconds = toml::find<int>(script, "throttle_seconds");
    } catch (const toml::syntax_error &e) {
      throw std::runtime_error("Syntax error in TOML file: " +
                               std::string(e.what()));
    } catch (const std::out_of_range &e) {
      throw std::runtime_error("range error in TOML file: " +
                               std::string(e.what()));
    } catch (const std::exception &e) {
      //    throw std::runtime_error("Error parsing TOML file: " +
      //    std::string(e.what()));
      throw std::runtime_error(
          "required key missing or mis-spelled - correct toml file");
      std::exit(EXIT_FAILURE);
    }
  }

  Program program_;
  Script script_;
};
