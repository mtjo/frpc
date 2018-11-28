#include "Frp.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <thread>
#include <regex>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <stdio.h>
#include "json/json.h"
#include "JSON.h"
#include "inifile.h"

//#include "boost/thread.hpp"
#include "Tools.h"

using std::string;
using std::thread;


#define BUF_SIZE 256

Frp::Frp() {
}

void startFrpc() {
    std::string output = Tools::runCommand("/frp/frpc_autorun.sh");
}

void killAutorun() {
    Tools::runCommand("sleep 2 && killall \"frpc_autorun.sh\">>./shell.log");
}


void
Frp::onLaunched(const std::vector <std::string> &parameters) {
    std::string frpcVersion = Tools::runCommand("/frp/frpc -v");
    Tools::saveData("frpcVersion",frpcVersion);
    std::thread subthread(startFrpc);
    subthread.detach();

    std::thread killthread(killAutorun);
    killthread.detach();
};


std::string
Frp::onParameterRecieved(const std::string &params) {
    std::string method = Tools::getParamsByKey(params, "method");
    JSONObject data;
    if (method == "") {
        return JSONObject::error(999, "method can not be null");
    } else if (method == "saveFrpcConfig") {
        std::string type = Tools::getParamsByKey(params, "type");
        Tools::saveData("configType", type);
        if (type == "user") {
            std::string jsondata = Tools::getParams(params);

            FILE *fp = NULL;
            fp = fopen("/etc/frpc_user_config.ini", "w+");
            fputs(jsondata.data(), fp);
            fclose(fp);
            return JSONObject::success();

        } else if (type == "base") {
            const char *ch = params.data();
            struct json_object *jsonObject = NULL;
            jsonObject = json_tokener_parse(ch);
            if ((long) jsonObject > 0) {/**Json格式无错误**/
                jsonObject = json_object_object_get(jsonObject, "data");
                saveConfig(jsonObject);
            }
            json_object_put(jsonObject);
            return JSONObject::success();
        }
        return JSONObject::error(1, "save's type missing");
    } else if (method == "getFrpcConfig") {
        std::string type = Tools::getParamsByKey(params, "type");
        string config = "";
        if (type == "base") {
            config = Tools::runCommand("cat /etc/frpc_config.ini");
        } else if (type == "user") {
            config = Tools::runCommand("cat /etc/frpc_user_config.ini");
        }
        return JSONObject::success(config);
    } else if (method == "getFrpcPid") {
        //std::string pid = Tools::runCommand("ps |grep 'frp/frpc'|grep -v 'grep'|grep -v '/bin/sh -c'|grep -v 'frpc_autorun.sh'|awk '{print $1}'");
        std::string pid = Tools::runCommand("pidof frpc");
        Tools::runCommand("echo "+pid+">pid");
        return JSONObject::success(pid);
    } else if (method == "runFrpc") {
        Tools::saveData("runStatus", "1");
        runFrpc();
        std::string pid = Tools::runCommand("pidof frpc");
        return JSONObject::success(pid);
    } else if (method == "stopFrpc") {
        Tools::saveData("runStatus", "0");
        stopFrpc();
        return JSONObject::success();
    } else if (method == "restartFrpc") {
        stopFrpc();
        runFrpc();
        std::string pid = Tools::runCommand("pidof frpc");
        return JSONObject::success(pid);
    } else if (method == "showFrpcLogFile") {
        std::string file = "";
        std::string configType = Tools::getData("configType");
        std::string logFile = "";
        if (configType == "base") {
            file = "/etc/frpc_config.ini";
        } else {
            file = "/etc/frpc_user_config.ini";
        }
        char value[32]={0};
        read_profile_string("common","log_file",value, sizeof(value),"/dev/null",file.data());

        logFile = value;
        std::string logStr ="日志文件不存在！！！";
        if (logFile != "/dev/null") {
            logStr = Tools::runCommand("cat "+logFile);
        }
        return JSONObject::success(logStr);
    } else if (method == "clearFrpcLogFile") {
        std::string file = "";
        std::string configType = Tools::getData("configType");
        std::string logFile = "";
        if (configType == "base") {
            file = "/etc/frpc_config.ini";
        } else {
            file = "/etc/frpc_user_config.ini";
        }
        char value[32]={0};
        read_profile_string("common","log_file",value, sizeof(value),"/dev/null",file.data());

        logFile = value;
        if (logFile != "/dev/null") {
            Tools::runCommand("echo ''>" +logFile);
        }
        return JSONObject::success();
    }

    return JSONObject::error(1, "parameter missing");


}


void
Frp::saveConfig(struct json_object *configData) {
    Tools::runCommand("rm -f etc/frpc_config.ini");
    const char *file = "etc/frpc_config.ini";
    json_object_object_foreach(configData, section, val)
    {
        json_object_object_foreach(val, key, value)
        {
            write_profile_string(section, key, json_object_get_string(value), file);
        }
    }
}

void Frp::runFrpc() {
    std::string runStatus = Tools::getData("runStatus");
    std::string configType = Tools::getData("configType");


    FILE *fp = NULL;
    fp = fopen("/frp/frpc_autorun.sh", "w+");
    fputs("#!/bin/ash\n", fp);
    fputs("PID=`pidof frpc`\n", fp);
    fputs("if [ -z $PID ]; then \n", fp);

    if (configType == "base") {
        fputs("/frp/frpc -c /etc/frpc_config.ini &>/dev/null\n", fp);
    } else {
        fputs("/frp/frpc -c /etc/frpc_user_config.ini &>/dev/null\n", fp);
    }
    fputs("echo \"on\"\n", fp);
    fputs("fi\n", fp);
    fclose(fp);

    if (runStatus == "1") {
        std::thread subthread(startFrpc);
        subthread.detach();
//
        std::thread killthread(killAutorun);
        killthread.detach();

    }
}

void Frp::stopFrpc() {
    Tools::runCommand("killall frp/frpc");
    Tools::runCommand("killall frp/frpc_autorun.sh");

    FILE *fp = NULL;

    fp = fopen("/frp/frpc_autorun.sh", "w+");
    fputs("#!/bin/ash\n", fp);
    fputs("echo \"off\"\n", fp);
    fputs("exit\n", fp);

    fclose(fp);

}


Frp frp;
