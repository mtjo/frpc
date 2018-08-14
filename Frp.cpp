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

#define BUF_SIZE 256

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
        std::string ps = exec("ps -w|grep frp/frpc");

        data.put("version", version);
        data.put("ps", ps);

        return JSONObject::success(data);


    } else if (method == "runFrpc") {
        system("./frp/frpc -c ./frp/frpc_full.ini &>/dev/null");
        return JSONObject::success();
    } else if (method == "stopFrpc") {
        system("killall frp/frpc");
        return JSONObject::success();
    }

    return JSONObject::error(1, "parameter missing");


}

void Frp::config() {
    const char *file = "myconfig.ini";
    const char *section = "student";
    const char *name_key = "name";
    const char *age_key = "age";
    char name[BUF_SIZE] = {0};
    int age;

    //write name key value pair
    if (!write_profile_string(section, name_key, "Tony", file)) {
        printf("write name pair to ini file fail\n");

    }

    //write age key value pair
    if (!write_profile_string(section, age_key, "20", file)) {
        printf("write age pair to ini file fail\n");

    }

    printf("[%s]\n", section);
    //read string pair, test read string value
    if (!read_profile_string(section, name_key, name, BUF_SIZE, "", file)) {
        printf("read ini file fail\n");
    } else {
        printf("%s=%s\n", name_key, name);
    }

    //read age pair, test read int value.
    //if read fail, return default value
    age = read_profile_int(section, age_key, 0, file);
    printf("%s=%d\n", age_key, age);
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


Frp frp;
