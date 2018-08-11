/**
 * @author Baiming Ma (mabaiming@xiaomi.com)
 * @date 2015-06-04
 */

#include "TaskWatcher.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "SystemState.h"
#include "JSON.h"
#include "INIParser.h"
#include<unistd.h>
#include<stdlib.h>


bool
TaskWatcher::parseValue(
  std::string& value, const std::string& content, const std::string& key) {
  std::string prefix = key + "=";
  size_t index = content.find(prefix);
  if (index == std::string::npos) {
    return false;
  }
  size_t start = index + prefix.size();
  if (start >= content.size()) {
    return false;
  }
  size_t end = content.find("&", start);
  value = (
    end == std::string::npos
    ? content.substr(start)
    : content.substr(start, end - start)
  );
  return true;
}

bool
TaskWatcher::parseValue(
  int32_t& value, const std::string& content, const std::string& key) {
  std::cerr << "content " << content << ", key: " << key << std::endl;
  std::string prefix = key + "=";
  size_t index = content.find(prefix);
  if (index == std::string::npos) {
    return false;
  }
  size_t start = index + prefix.size();
  if (start >= content.size()) {
    return false;
  }
  auto stringToInt = [&](const std::string& str, size_t i) {
    int sign = 1;
    if (str[0] == '-') {
      sign = -1;
      ++i;
    }
    int value = 0;
    while (i < str.size() && isdigit(str[i])) {
      value *= 10;
      value += str[i] - '0';
      ++i;
    }
    return value * sign;
  };
  value = stringToInt(content, start);
  return true;
}



std::string
TaskWatcher::onParameterRecieved(const std::string& params) {
  using namespace xiaomi::miwifi;

  std::cout << "params: " << params << std::endl;

  int32_t delay = 50;
  parseValue(delay, params, "delay");
  delay = std::min(std::max(50, delay), 10000);

  int32_t count = 10;
  parseValue(count, params, "count");
  count = std::min(std::max(5, count), 500);

  std::string orderKey = "cpu";
  parseValue(orderKey, params, "orderKey");

  bool asc = false;
  std::string ascString;
  if (parseValue(ascString, params, "asc") && ascString == "true") {
    asc = true;
  }
  std::cerr
    << "delay: " << delay
    << ", count: " << count
    << ", order key: " << orderKey
    << ", asc: " << asc
    << std::endl;

  SystemState systemState;
  if (!SystemState::getSystemState(systemState, delay)) {
    std::cerr << "unable to access system state";
    return JSONObject::error(-1, "unable to access system state");
  }

  JSONObject summary;

  summary.put("cpuUsage", systemState.getCPUUsagePercent());
  summary.put("memoryTotal", systemState.getMemoryInfo().getTotal());
  summary.put("memoryUsed", systemState.getMemoryInfo().getUsed());
  summary.put("processCount", systemState.getProcessCount());

  std::vector<ProcessInfo> processList =  systemState.getProcessList();

  if (orderKey == "pid") {
    sort(
      processList.begin(),
      processList.end(),
      [&](const ProcessInfo& a, const ProcessInfo& b) {
        return a.getID() < b.getID();
      }
    );
  } else if (orderKey == "ppid") {
    sort(
      processList.begin(),
      processList.end(),
      [&](const ProcessInfo& a, const ProcessInfo& b) {
        return a.getParentID() < b.getParentID();
      }
    );
  } else if (orderKey == "cpu") {
    sort(
      processList.begin(),
      processList.end(),
      [&](const ProcessInfo& a, const ProcessInfo& b) {
        return  a.getCPUUsage() < b.getCPUUsage();
      }
    );
  } else if (orderKey == "priority") {
    sort(
      processList.begin(),
      processList.end(),
      [&](const ProcessInfo& a, const ProcessInfo& b) {
        return  a.getPriority() < b.getPriority();
      }
    );
  } else if (orderKey == "nice") {
    sort(
      processList.begin(),
      processList.end(),
      [&](const ProcessInfo& a, const ProcessInfo& b) {
        return  a.getNice() < b.getNice();
      }
    );
  } else if (orderKey == "threadCount") {
    sort(
      processList.begin(),
      processList.end(),
      [&](const ProcessInfo& a, const ProcessInfo& b) {
        return  a.getThreadCount() < b.getThreadCount();
      }
    );
  } else if (orderKey == "physicalMemory") {
    sort(
      processList.begin(),
      processList.end(),
      [&](const ProcessInfo& a, const ProcessInfo& b) {
        return a.getPhysicalMemorySize() < b.getPhysicalMemorySize();
      }
    );
  } else if (orderKey == "virtualMemory") {
    sort(
      processList.begin(),
      processList.end(),
      [&](const ProcessInfo& a, const ProcessInfo& b) {
        return a.getVirtualMemorySize() < b.getVirtualMemorySize();
        }
    );
  } else if (orderKey == "name") {
    sort(
      processList.begin(),
      processList.end(),
      [&](const ProcessInfo& a, const ProcessInfo& b) {
        return a.getName() < b.getName();
      }
    );
  } else if (orderKey == "state") {
    sort(
      processList.begin(),
      processList.end(),
      [&](const ProcessInfo& a, const ProcessInfo& b) {
        return a.getState() < b.getState();
      }
    );
  } else {
    std::cerr << "unrecognized order key :" << orderKey << std::endl;
    return JSONObject::error(-1, "unrecognized order key :" + orderKey);
  }

  count = std::min(count, static_cast<int>(processList.size()));
  auto processToJSON = [&](const ProcessInfo& processInfo) {
    JSONObject item;
    item.put("pid", processInfo.getID());
    item.put("name", processInfo.getName());
    item.put("state", std::string().assign(1, processInfo.getState()));
    item.put("ppid", processInfo.getParentID());
    item.put("cpu", processInfo.getCPUUsage());
    item.put("priority", processInfo.getPriority());
    item.put("nice", processInfo.getNice());
    item.put("threadCount", processInfo.getThreadCount());
    item.put("virtualMemory", processInfo.getVirtualMemorySize());
    item.put("physicalMemory", processInfo.getPhysicalMemorySize());
    return item;
  };

  JSONArray processArray;
  if (asc) {
    auto it = processList.begin();
    while (count-- > 0) {
      JSONObject json = processToJSON(*it);
      processArray.append(json);
      ++it;
    }
  } else {
    auto it = processList.rbegin();
    while (count-- > 0) {
      JSONObject json = processToJSON(*it);
      processArray.append(json);
      ++it;
    }
  }

  JSONObject data;
  data.put("summary", summary);
  data.put("processList", processArray);
  return JSONObject::success(data);
}

TaskWatcher taskWatcher;
