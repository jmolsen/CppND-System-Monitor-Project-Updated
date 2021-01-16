#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"
#include "system.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

Processor& System::Cpu() { return cpu_; }

vector<Process>& System::Processes() {
  processes_.clear();
  vector<int> pids = LinuxParser::Pids();
  for (int pid : pids) {
    string name = LinuxParser::User(LinuxParser::Uid(pid));
    string command = LinuxParser::Command(pid);
    processes_.emplace_back(Process{pid, name, command});
  }
  std::sort(processes_.begin(), processes_.end());
  return processes_;
}

std::string System::Kernel() {
  if (kernel_.empty()) {
    kernel_ = LinuxParser::Kernel();
  }
  return kernel_;
}

float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

std::string System::OperatingSystem() {
  if (operatingSystem_.empty()) {
    operatingSystem_ = LinuxParser::OperatingSystem();
  }
  return operatingSystem_;
}

int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

long System::UpTime() {
  long uptime = LinuxParser::UpTime();

  // If kernel version < 2.6, then uptime stored as jiffies and need to be divided by sysconf(_SC_CLK_TCK)
  string fullKernel = Kernel();
  size_t pos = 0;
  // Get index of second instance of "."
  for (int i = 0; i < 2; i++) {
    pos = fullKernel.find(".", ++pos);
    if (pos == string::npos) {
      pos = 2;
      break;
    }
  }
  float kernelVersion = stof(fullKernel.substr(0, pos));
  if (kernelVersion < 2.6) {
    uptime /= sysconf(_SC_CLK_TCK);
  }

  return uptime;
}