#include "Frp.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
    const char *ch = params.data();
//    params>>ch;
    struct json_object *obj_pck = NULL;
    obj_pck = json_tokener_parse(ch);
    ((long) obj_pck < 0);  /**Json格式错误**/
    config();
    JSONObject object;
    object.put("data", json_object_object_length(obj_pck));

    std::string keys = "";
    json_object_object_foreach(obj_pck, key, val)
    {

        printf("\t%s: %s\n", key, json_object_to_json_string(val));
        keys += key;
    }
    object.put("keys", keys);
    object.put("method",method);
    return JSONObject::success(object);
    //return json_object_to_json_string(obj_pck);

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
    struct json_object *obj_pck = NULL;
    obj_pck = json_tokener_parse(ch);
    std::string method;
    if ((long) obj_pck < 0) {/**Json格式错误**/
        return "";
    } else {
        struct json_object *result_object = NULL;
        result_object = json_object_object_get(obj_pck, "method");
        return json_object_to_json_string(result_object);
    }
}


Frp frp;
