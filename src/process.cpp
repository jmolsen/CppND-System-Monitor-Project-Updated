#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid, std::string user, std::string command) : pid_(pid), user_(user), command_(command) {}

int Process::Pid() const { return pid_; }

float Process::CpuUtilization() const {
  float seconds = static_cast<float>(LinuxParser::UpTime()) - static_cast<float>(LinuxParser::UpTime(Pid()));
  return ((LinuxParser::ActiveJiffies(Pid()) / sysconf(_SC_CLK_TCK)) / seconds);
  //return (LinuxParser::ActiveJiffies(Pid()) / seconds);
}

string Process::Command() const { return command_; }

string Process::Ram() const { return LinuxParser::Ram(Pid()); }

string Process::User() const { return user_; }

long int Process::UpTime() const { return LinuxParser::UpTime() - LinuxParser::UpTime(Pid()); }

bool Process::operator<(Process const& a) const { return a.CpuUtilization() < CpuUtilization(); }