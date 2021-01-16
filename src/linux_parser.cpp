#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::stol;
using std::string;
using std::to_string;
using std::vector;

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream filestream(kProcDirectory + kVersionFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() {
  string key, value;
  float memTotal = 0.0, memFree = 0.0;
  float buffers = 0.0;
  float cached = 0.0, shmem = 0.0, sreclaimable = 0.0;
  string line;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "MemTotal:") {
        memTotal = stof(value);
      } else if (key == "MemFree:") {
        memFree = stof(value);
      } else if (key == "Buffers:") {
        buffers = stof(value);
      } else if (key == "Cached:") {
        cached = stof(value);
      } else if (key == "Shmem:") {
        shmem = stof(value);
      } else if (key == "SReclaimable:") {
        sreclaimable = stof(value);
      }
    }
  }
  float totalUsedMemory = memTotal - memFree;
  float cachedMemory = cached + sreclaimable - shmem;
  float nonCacheOrBufferMemory = totalUsedMemory - (buffers + cachedMemory);
  return nonCacheOrBufferMemory / memTotal;
}

long LinuxParser::UpTime() {
  string uptimeStr;
  long uptime = 0;
  string line;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> uptimeStr;
    uptime = static_cast<long>(stof(uptimeStr));
  }
  return uptime;
}

long LinuxParser::Jiffies() { return ActiveJiffies() + IdleJiffies(); }

long LinuxParser::ActiveJiffies(int pid) {
  string comm, state, ppid, pgrp, session, tty_nr, tpgid, flags, minflt, cminflt, majflt, cmajflt;
  string utime, stime, cutime, cstime, priority, nice, num_threads, itrealvalue, starttime;
  long activeJiffies = 0;
  string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt;
    linestream >> utime >> stime >> cutime >> cstime >> priority >> nice >> num_threads >> itrealvalue >> starttime;

    // Sum user mode + kernel mode time
    activeJiffies = stol(utime) + stol(stime);
    // Add in wait time for child processes in user mode and kernel mode
    activeJiffies += stol(cutime) + stol(cstime);
  }
  return activeJiffies;
}

long LinuxParser::ActiveJiffies() {
  vector<string> cpuUtilization = CpuUtilization();
  long UserTime = stol(cpuUtilization[kUser_]);
  long NiceTime = stol(cpuUtilization[kNice_]);
  long SystemTime = stol(cpuUtilization[kSystem_]);
  long IrqTime = stol(cpuUtilization[kIRQ_]);
  long SoftIrqTime = stol(cpuUtilization[kSoftIRQ_]);
  long StealTime = stol(cpuUtilization[kSteal_]);
  long GuestTime = stol(cpuUtilization[kGuest_]);
  long GuestNiceTime = stol(cpuUtilization[kGuestNice_]);
  // UserTime and NiceTime include the associated Guest times
  UserTime -= GuestTime;
  NiceTime -= GuestNiceTime;
  // We could have not subtracted the Guest times above and thus wouldn't have needed
  // to add them below, but this make it more clear what all is included
  return UserTime + NiceTime + SystemTime + IrqTime + SoftIrqTime + StealTime + GuestTime + GuestNiceTime;
}

long LinuxParser::IdleJiffies() {
  vector<string> cpuUtilization = CpuUtilization();
  long IdleTime = stol(cpuUtilization[kIdle_]);
  long IoWaitTime = stol(cpuUtilization[kIOwait_]);
  return IdleTime + IoWaitTime;
}

vector<string> LinuxParser::CpuUtilization() {
  vector<string> cpuUtilization;
  string line;
  string value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    // Iterate over all CPU lines to get data for aggregate and individual CPUs
    while (std::getline(filestream, line) && line.rfind("cpu", 0) == 0) {
      std::istringstream linestream(line);
      linestream >> value; // take off label
      while (linestream >> value) {
        cpuUtilization.emplace_back(value);
      }
    }
  } else {
    cpuUtilization = {"0", "0", "0", "0", "0", "0", "0", "0", "0", "0"};
  }
  return cpuUtilization;
}

int LinuxParser::TotalProcesses() {
  string key, value;
  int totalProcesses = 0;
  string line;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "processes") {
        totalProcesses = stoi(value);
      }
    }
  }
  return totalProcesses;
}

int LinuxParser::RunningProcesses() {
  string key, value;
  int runningProcesses = 0;
  string line;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "procs_running") {
        runningProcesses = stoi(value);
      }
    }
  }
  return runningProcesses;
}

string LinuxParser::Command(int pid) {
  string name, command;
  string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> name >> command;
  }
  return command;
}

string LinuxParser::Ram(int pid) {
  string key, value;
  long ram;
  string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "VmSize:") {
        // convert from kB to MB
        ram = static_cast<long>(stof(value) / 1000);
        break;
      }
    }
  }
  return to_string(ram);
}

string LinuxParser::Uid(int pid) {
  string key;
  string real_uid, effective_uid;
  string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "Uid:") {
        linestream >> real_uid >> effective_uid;
        break;
      }
    }
  }
  return effective_uid;
}

string LinuxParser::User(string userId) {
  string user, x, uid;
  string line;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);

      linestream >> user >> x >> uid;
      if (uid == userId) {
        break;
      }
    }
  }
  return user;
}

long LinuxParser::UpTime(int pid) {
  string comm, state, ppid, pgrp, session, tty_nr, tpgid, flags, minflt, cminflt, majflt, cmajflt;
  string utime, stime, cutime, cstime, priority, nice, num_threads, itrealvalue, starttime;
  long uptime = 0;
  string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt;
    linestream >> utime >> stime >> cutime >> cstime >> priority >> nice >> num_threads >> itrealvalue >> starttime;
    uptime = static_cast<long>(stol(starttime) / sysconf(_SC_CLK_TCK));
  }
  return uptime;
}