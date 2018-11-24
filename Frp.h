#pragma once

#include <string>

#include "MRApp.h"

class Frp : public MRApp {
public:
    Frp();
    virtual ~Frp(){};

    void config();
    void runFrpc();
    void stopFrpc();

    void saveConfig(struct json_object *configData);

    virtual void onLaunched(const std::vector<std::string> &parameters);
    virtual std::string onParameterRecieved(const std::string &params);
};
