#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "process_parser.h"
#include "system_parser.h"

using std::string;
using std::to_string;
using std::vector;

// BONUS: Upgrade this to use C++17 std::filesystem
vector<string> ProcessParser::Pids() {
  vector<string> pids;
  DIR* directory = opendir("/proc");
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        pids.push_back(filename);
      }
    }
  }
  closedir(directory);
  return pids;
}

string ProcessParser::VmSize(string pid) {
  string token;
  std::ifstream stream(Path::base + pid + Path::status);
  if (stream.is_open()) {
    while (stream >> token) {
      if (token == "VmSize:") {
        if (stream >> token) return std::to_string(stoi(token) / 1024);
      }
    }
  }
  return string("0");
}

string ProcessParser::Cmdline(string pid) {
  string line;
  std::ifstream stream(Path::base + pid + Path::cmdline);
  if (stream.is_open()) {
    string line;
    std::getline(stream, line);
    return line;
  }
  return "";
}

long ProcessParser::Jiffies(string pid) {
  string line, token;
  vector<string> values;
  std::ifstream filestream(Path::base + pid + Path::stat);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (linestream >> token) {
      values.push_back(token);
    }
  }
  long jiffies{0};
  if (values.size() > 21) {
    long user = stol(values[13]);
    long kernel = stol(values[14]);
    long children_user = stol(values[15]);
    long children_kernel = stol(values[16]);
    jiffies = user + kernel + children_user + children_kernel;
  }
  return jiffies;
}

string ProcessParser::UpTime(string pid) {
  string token;
  std::ifstream stream(Path::base + pid + Path::stat);
  if (stream.is_open()) {
    for (int i = 0; stream >> token; ++i)
      if (i == 13) {
        int time{stoi(token)};
        time /= sysconf(_SC_CLK_TCK);
        return to_string(time);
      }
  }
  return "NA";
}

string ProcessParser::Uid(string pid) {
  string token;
  std::ifstream stream(Path::base + pid + Path::status);
  if (stream.is_open()) {
    while (stream >> token) {
      if (token == "Uid:") {
        if (stream >> token) return token;
      }
    }
  }
  return string("");
}

string ProcessParser::User(string pid) {
  std::ifstream stream(Path::users);
  if (stream.is_open()) {
    string line;
    string token = "x:" + ProcessParser::Uid(pid);
    while (std::getline(stream, line)) {
      auto marker = line.find(token);
      if (marker != string::npos) {
        return line.substr(0, marker - 1);
      }
    }
  }
  return "0";
}