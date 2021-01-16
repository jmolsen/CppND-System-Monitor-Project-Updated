#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();

 private:
  long TotalIdleJiffies_{0};
  long TotalJiffies_{0};
};

#endif