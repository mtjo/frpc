/**
 * @author Baiming Ma (mabaiming@xiaomi.com)
 * @date 2015-06-04
 */

#pragma once

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

namespace xiaomi {
namespace miwifi {

class CPUUsage {
public:
  static bool getCPUUsage(CPUUsage& cpuUsage);

  CPUUsage() :
    user_(0),
    nice_(0),
    system_(0),
    idle_(0),
    ioWait_(0),
    irq_(0),
    softIRQ_(0),
    steal_(0) {}

  int64_t getUser() const {return user_;}
  int64_t getNice() const {return nice_;}
  int64_t getSystem() const {return system_;}
  int64_t getIdle() const {return idle_;}
  int64_t getIOWait() const {return ioWait_;}
  int64_t getIRQ() const {return irq_;}
  int64_t getSoftIRQ() const {return softIRQ_;}
  int64_t getSteal() const {return steal_;}

private:
  int64_t user_;
  int64_t nice_;
  int64_t system_;
  int64_t idle_;
  int64_t ioWait_;
  int64_t irq_;
  int64_t softIRQ_;
  int64_t steal_;
};

class MemoryInfo {
public:
  static bool getMemoryInfo(MemoryInfo& memoryInfo);

  MemoryInfo() :
    buffer_(0),
    free_(0),
    freeSwap_(0),
    total_(0),
    totalSwap_(0) {}

  int64_t getBuffer() const {return buffer_;}
  int64_t getFree() const {return free_;}
  int64_t getFreeSwap() const {return freeSwap_;}
  int64_t getTotal() const {return total_;}
  int64_t getTotalSwap() const {return totalSwap_;}
  int64_t getUsed() const {return total_ - free_;}
  int64_t getUsedSwap() const {return totalSwap_ - freeSwap_;}

private:
  int64_t buffer_;
  int64_t free_;
  int64_t freeSwap_;
  int64_t total_;
  int64_t totalSwap_;
};


class ProcessInfo {
public:
  static const int32_t sTaskCommandLength;

  static bool getStat(std::string& info, const std::string& pid);
  static bool parse(ProcessInfo& processInfo, const std::string& line);

  ProcessInfo() :
    id_(0),
    name_(),
    state_(0),
    parentID_(0),
    cpuTime_(0),
    cpuUsage_(0),
    priority_(0),
    nice_(0),
    threadCount_(0),
    startTime_(0),
    virtualMemorySize_(0),
    physicalMemorySize_(0) {}

  int32_t getID() const {return id_;}
  const std::string& getName() const {return name_;}
  char getState() const {return state_;}
  int32_t getParentID() const {return parentID_;}
  int64_t getCPUTime() const {return cpuTime_;}
  int32_t getCPUUsage() const {return cpuUsage_;}
  int32_t getPriority() const {return priority_;}
  int32_t getNice() const {return nice_;}
  int32_t getThreadCount() const {return threadCount_;}
  int64_t getStartTime() const {return startTime_;}
  int64_t getVirtualMemorySize() const {return virtualMemorySize_;}
  int64_t getPhysicalMemorySize() const {return physicalMemorySize_;}

  void setCpuUsage(int32_t usage) {cpuUsage_ = usage;}

private:
  int32_t id_;
  std::string name_;
  char state_;
  int32_t parentID_;
  int64_t cpuTime_;
  int32_t cpuUsage_;
  int32_t priority_;
  int32_t nice_;
  int32_t threadCount_;
  int64_t startTime_;
  int64_t virtualMemorySize_;
  int64_t physicalMemorySize_;
};

class SystemState {
public:
  static const int32_t sDefaultDelayInMilliseconds;
  static const int32_t sMaxDelayInMilliseconds;

  static bool getPIDList(std::vector<std::string>& processIDList);

  static bool getSystemState(
    SystemState& systemState, int32_t delayInMilliseconds
  );

  SystemState() :
    cpuUsage_(0),
    memoryInfo_(),
    processCount_(0) {}

  int32_t getCPUUsagePercent() const {return cpuUsage_;}
  std::vector<ProcessInfo> getProcessList() const {return processList;}
  const MemoryInfo& getMemoryInfo() const {return memoryInfo_;}
  int32_t getProcessCount() const {return processCount_;}

private:
  int32_t cpuUsage_;
  std::vector<ProcessInfo> processList;
  MemoryInfo memoryInfo_;
  int32_t processCount_;
};

} // namespace miwifi
} // namespace xiaomi
