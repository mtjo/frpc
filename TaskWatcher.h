/**
 * @author Baiming Ma (mabaiming@xiaomi.com)
 * @date 2015-06-04
 */

#pragma once

#include <string>

#include "MRApp.h"

class TaskWatcher: public MRApp {
public:
  static bool parseValue(
    std::string& value, const std::string& content, const std::string& key
  );

  static bool parseValue(
    int32_t& value, const std::string& content, const std::string& key
  );

  TaskWatcher() {}
  virtual ~TaskWatcher() {}

  virtual void onLaunched(const std::vector<std::string>& parameters) {}
  virtual std::string onParameterRecieved(const std::string& params);
};
