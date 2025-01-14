#include "linux_parser.h"
#include "processor.h"

float Processor::Utilization() {
  // Get copies of previous values
  long PrevTotalIdle = TotalIdleJiffies_;
  long PrevTotal = TotalJiffies_;

  // Update values to current times
  TotalIdleJiffies_ = LinuxParser::IdleJiffies();
  TotalJiffies_ = LinuxParser::Jiffies();

  // Calculate change
  float deltaIdle = TotalIdleJiffies_ - PrevTotalIdle;
  float deltaTotal = TotalJiffies_ - PrevTotal;

  return (deltaTotal - deltaIdle) / deltaTotal;
}