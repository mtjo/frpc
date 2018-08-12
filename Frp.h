#pragma once

#include <string>

#include "MRApp.h"

class Frp : public MRApp {
public:
    Frp(){};

    virtual ~Frp(){};

    void config();
    std::string getMethod(const std::string &params);
    virtual void onLaunched(const std::vector<std::string> &parameters){};

    virtual std::string onParameterRecieved(const std::string &params);
};
