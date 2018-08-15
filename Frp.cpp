#include "Frp.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <regex>

using std::string;

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <stdio.h>
#include "json/json.h"
#include "JSON.h"
#include "inifile.h"
#include "DataTransfer.h"

using router::DataTransfer;


#define BUF_SIZE 256

Frp::Frp() {
}

void
Frp::onLaunched(const std::vector <std::string> &parameters) {
};


std::string exec(const char *cmd) {
    char buffer[128];
    std::string result = "";
    FILE *pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}


std::string
Frp::onParameterRecieved(const std::string &params) {
    std::string method = getMethod(params);
    JSONObject data;
    if (method == "") {
        return JSONObject::error(999, "method can not be null");
    } else if (method == "saveConfig") {
        data.put("method", method);
        std::string jsondata = getData(params);
        //data.put("data", jsondata);


        const char *ch = params.data();
        struct json_object *jsonObject = NULL;
        jsonObject = json_tokener_parse(ch);

        if ((long) jsonObject > 0) {/**Json格式无错误**/
            jsonObject = json_object_object_get(jsonObject, "data");
            saveConfig(jsonObject);

        }
        json_object_put(jsonObject);
        return JSONObject::success(data);
    } else if (method == "getStatus") {
        std::string version = exec("frp/frpc -v");

        std::string status = exec("ps |grep 'frp/frpc'|grep -v 'grep'|grep -v '/bin/sh -c'|awk '{print $1}'");
        exec("ps |grep 'frp/frpc'|grep -v 'grep'|grep -v '/bin/sh -c'|awk '{print $1}'>pid");
        data.put("version", version);
        data.put("status", status);

        return JSONObject::success(data);


    } else if (method == "runFrpc") {
        router::DataTransfer::saveData("run_status", "1");
        runFrpc();
        return JSONObject::success();
    } else if (method == "stopFrpc") {
        router::DataTransfer::saveData("run_status", "0");
        std::string run_status;
        router::ErrorCode::Code resCode = router::DataTransfer::getData("run_status", run_status);
        data.put("rescode", resCode);
        data.put("run_status", run_status);
        stopFrpc();
        return JSONObject::success(data);
    }

    return JSONObject::error(1, "parameter missing");


}

std::string
Frp::getMethod(const std::string &params) {
    const char *ch = params.data();
    struct json_object *jsonObject = NULL;
    jsonObject = json_tokener_parse(ch);
    std::string method = "";
    if ((long) jsonObject > 0) {/**Json格式无错误**/
        jsonObject = json_object_object_get(jsonObject, "method");
        method = json_object_get_string(jsonObject);
    }
    json_object_put(jsonObject);
    return method;
}


std::string
Frp::getData(const std::string &params) {
    const char *ch = params.data();
    struct json_object *jsonObject = NULL;
    jsonObject = json_tokener_parse(ch);
    std::string method = "";
    if ((long) jsonObject > 0) {/**Json格式无错误**/
        jsonObject = json_object_object_get(jsonObject, "data");
        method = json_object_get_string(jsonObject);
    }
    json_object_put(jsonObject);
    return method;
}


void
Frp::saveConfig(struct json_object *configData) {
    exec("rm -f etc/frpc_config.ini");
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
    std::string run_status;
    router::DataTransfer::getData("run_status", run_status);
    FILE *fp = NULL;

    fp = fopen("/frp/autorun.sh", "w+");
    fputs("#!/bin/ash\n", fp);
    fputs(".frp/frpc -c ./etc/frpc_config.ini &>/dev/null\n", fp);
    fputs("echo \"on\"\n", fp);
    fclose(fp);

    if (run_status == "1") {
        system("./frp/frpc -c ./etc/frpc_config.ini &>/dev/null");
    }
}

void Frp::stopFrpc() {

    FILE *fp = NULL;

    fp = fopen("/frp/autorun.sh", "w+");
    fputs("#!/bin/ash\n", fp);
    fputs("echo \"off\"\n", fp);
    fclose(fp);

    system("killall frp/frpc");
}

Frp frp;
