#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <format>

#include "yaml-cpp/yaml.h"

using namespace std;

int main()
{
    YAML::Node config = YAML::LoadFile("config.yaml");

    if (config["lastLogin"]) {
    std::cout << "Last logged in: " << config["lastLogin"].as<string>() << "\n";
    }

    const std::string username = config["username"].as<std::string>();
    const std::string password = config["password"].as<std::string>();
    // login(username, password);
    // config["lastLogin"] = getCurrentDateTime();
    cout << "user: " + username << endl;
    cout << "pswd: " + password << endl;

    using namespace std::chrono;
    auto now = system_clock::now();
    string date;
    date = std::format("{:%Y-%m-%d %H:%M:%S}", floor<std::chrono::seconds>(now));
    config["date"] = date;
    cout << "write the date to config.yaml: " << date << endl;
    std::ofstream fout("config.yaml");
    fout << config;
}