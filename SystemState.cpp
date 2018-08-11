/**
 * @author Baiming Ma (mabaiming@xiaomi.com)
 * @date 2015-06-04
 */

#include "SystemState.h"

#include <dirent.h>
#include <stdint.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

namespace xiaomi {
namespace miwifi {

bool
CPUUsage::getCPUUsage(CPUUsage& cpuUsage) {
  std::string cpu;
  std::ifstream fin("/proc/stat");
  if (!fin.is_open()) {
    return false;
  }
  fin
  >> cpu
  >> cpuUsage.user_
  >> cpuUsage.nice_
  >> cpuUsage.system_
  >> cpuUsage.idle_
  >> cpuUsage.ioWait_
  >> cpuUsage.irq_
  >> cpuUsage.softIRQ_
  >> cpuUsage.steal_;
  fin.close();
  return true;
}

bool
MemoryInfo::getMemoryInfo(MemoryInfo& memoryInfo) {
  struct sysinfo sysInfo;
  if (sysinfo(&sysInfo) != 0) {
    return false;
  }

  memoryInfo.total_ = sysInfo.totalram;
  memoryInfo.free_ = sysInfo.freeram;
  memoryInfo.buffer_ = sysInfo.bufferram;
  memoryInfo.totalSwap_ = sysInfo.totalswap;
  memoryInfo.freeSwap_ = sysInfo.freeswap;

  memoryInfo.total_ *= sysInfo.mem_unit;
  memoryInfo.free_ *= sysInfo.mem_unit;
  memoryInfo.buffer_ *= sysInfo.mem_unit;
  memoryInfo.totalSwap_ *= sysInfo.mem_unit;
  memoryInfo.free_ *= sysInfo.mem_unit;

  return true;
}

const int32_t ProcessInfo::sTaskCommandLength = 16;

bool
ProcessInfo::getStat(std::string& info, const std::string& pid) {
  std::string fileName = "/proc/" + pid + "/stat";
  std::ifstream fin(fileName);
  if (!fin.is_open()) {
    return false;
  }
  std::getline(fin, info);
  fin.close();
  return true;
}

bool
ProcessInfo::parse(ProcessInfo& processInfo, const std::string& line) {
  // /proc/pid/stat has the following fields we care about:
  // char pid[16]; // 0 %d here used as string
  // char name[20]; 1 (%s)
  // char state; // 2 %c
  // char ppid[16]; // 3 %d here used as string
  // unsigned long int utime; //13 %lu
  // unsigned long int stime; // 14 %lu
  // long int cutime; // 15 %ld
  // long int cstime; // 16 %ld
  // long int priority; // 17 %ld
  // long int nice; // 18 %ld
  // long int threadCount; // 19 %ld
  // unsigned long long int startTime; // 21 %llu
  // unsigned long int vsize; // 22 %lu
  // long int rss; // 23 %ld

  // find parentheses
  auto left = line.find('(');
  if (left == std::string::npos) {
    return false;
  }
  auto right = line.rfind(')', left + sTaskCommandLength);
  if (right == std::string::npos) {
    return false;
  }

  int pid;
  char state;
  int ppid;
  unsigned long int utime;
  unsigned long int stime;
  long int cutime;
  long int cstime;
  long int priority;
  long int nice;
  long int threadCount;
  unsigned long long int startTime;
  unsigned long int vsize;
  long int rss;

  int result = sscanf(line.c_str(), "%d", &pid);
  if (result == -1) {
    return false;
  }

  result = sscanf(
    // + 2, effectively skipping ") "
    line.c_str() + right + 2,
    "%c %d "
      "%*s %*s %*s %*s %*s %*s %*s %*s %*s "
      "%lu %lu %ld %ld %ld %ld %ld "
      "%*s "
      "%llu %lu %ld",
    &state, &ppid,
    &utime, &stime, &cutime, &cstime, &priority, &nice, &threadCount,
    &startTime, &vsize, &rss
  );

  if (result == -1) {
    return false;
  }

  processInfo.id_ = pid;
  processInfo.name_.assign(line.begin() + left + 1, line.begin() + right);
  processInfo.state_ = state;
  processInfo.parentID_ = ppid;
  processInfo.cpuTime_ = utime;
  processInfo.cpuTime_ += stime;
  // The following two lines are not used in top program.
  // So we comment them out for now.
  // processInfo.cpuTime_ += cutime; skip cutime
  // processInfo.cpuTime_ += cstime; skip cstime

  processInfo.priority_ = priority;
  processInfo.nice_ = nice;
  processInfo.threadCount_ = threadCount;
  processInfo.startTime_ = startTime;
  processInfo.virtualMemorySize_ = vsize;
  processInfo.physicalMemorySize_ = rss * ::getpagesize();

  return true;
}

const int32_t SystemState::sDefaultDelayInMilliseconds = 50;
const int32_t SystemState::sMaxDelayInMilliseconds = 5000;

bool
SystemState::getPIDList(std::vector<std::string>& processIDList) {
  DIR *dir = opendir("/proc");
  if (dir == NULL) {
    return false;
  }
  struct dirent* ptr;
  while (ptr = readdir(dir)) {
    if (ptr->d_name[0] > '0' && ptr->d_name[0] <= '9') {
      processIDList.push_back(ptr->d_name);
    }
  }
  return true;
}

bool
SystemState::getSystemState(
  SystemState& systemState, int32_t delayInMilliseconds) {
  if (delayInMilliseconds < 0) {
    delayInMilliseconds = sDefaultDelayInMilliseconds;
  } else if(delayInMilliseconds > sMaxDelayInMilliseconds) {
    delayInMilliseconds = sMaxDelayInMilliseconds;
  }

  std::vector<std::string> processIDList;
  if (!getPIDList(processIDList)) {
    return false;
  }

  CPUUsage lastCPUUsage;
  if (!CPUUsage::getCPUUsage(lastCPUUsage)) {
    return false;
  }

  std::vector<std::string> lastProcessInfoList;
  for (const std::string& processID : processIDList) {
    std::string result;
    if (ProcessInfo::getStat(result, processID)) {
      lastProcessInfoList.push_back(std::move(result));
    }
  }

  ::usleep(delayInMilliseconds * 1000);

  std::vector<std::string> currentProcessInfoList;
  for (const std::string& processID : processIDList) {
    std::string result;
    if (ProcessInfo::getStat(result, processID)) {
      currentProcessInfoList.push_back(std::move(result));
    }
  }

  CPUUsage currentCPUUsage;
  if (!CPUUsage::getCPUUsage(currentCPUUsage)) {
    return false;
  }

  int64_t idleDiff = currentCPUUsage.getIdle() - lastCPUUsage.getIdle();
  int64_t niceDiff = currentCPUUsage.getNice() - lastCPUUsage.getNice();
  int64_t systemDiff = currentCPUUsage.getSystem() - lastCPUUsage.getSystem();
  int64_t userDiff = currentCPUUsage.getUser() - lastCPUUsage.getUser();
  int64_t ioWaitDiff = currentCPUUsage.getIOWait() - lastCPUUsage.getIOWait();
  int64_t irqDiff = currentCPUUsage.getIRQ() - lastCPUUsage.getIRQ();
  int64_t softIRQDiff =
    currentCPUUsage.getSoftIRQ() - lastCPUUsage.getSoftIRQ();
  int64_t stealDiff = currentCPUUsage.getSteal() - lastCPUUsage.getSteal();
  int64_t totalTimeDiff = systemDiff + niceDiff + userDiff + idleDiff +
                          ioWaitDiff + irqDiff + softIRQDiff + stealDiff;

  bool failure =
    idleDiff < 0 || niceDiff < 0 || systemDiff < 0 || userDiff < 0 ||
    ioWaitDiff < 0 || irqDiff < 0 || softIRQDiff < 0 || stealDiff < 0 ||
    totalTimeDiff <= 0;

  if (failure) {
    return false;
  }

  systemState.cpuUsage_ = 1000 - idleDiff * 1000 / totalTimeDiff;

  std::map<int32_t, ProcessInfo> lastProcessInfoMap;
  for (const std::string& processStat : lastProcessInfoList) {
    ProcessInfo processInfo;
    if (ProcessInfo::parse(processInfo, processStat)) {
      lastProcessInfoMap.insert(
        std::make_pair(processInfo.getID(), std::move(processInfo))
      );
    }
  }

  std::vector<ProcessInfo> processList;
  const auto& end = lastProcessInfoMap.end();
  for (const std::string& processStat : currentProcessInfoList) {
    ProcessInfo processInfo;
    if (processInfo.parse(processInfo, processStat)) {
      auto it = lastProcessInfoMap.find(processInfo.getID());
      if (it == end) {
        continue;
      }
      ProcessInfo& lastProcessInfo = it->second;
      int64_t cpuDiff = processInfo.getCPUTime() - lastProcessInfo.getCPUTime();
      int64_t cpuUsage = cpuDiff * 1000 / totalTimeDiff;
      processInfo.setCpuUsage(static_cast<int32_t>(cpuUsage));
      processList.push_back(std::move(processInfo));
    }
  }

  systemState.processCount_ = processList.size();

  systemState.processList = std::move(processList);

  return MemoryInfo::getMemoryInfo(systemState.memoryInfo_);
}

} // namespace miwifi
} // namespace xiaomi
