#pragma once
//Hukou_Helper.h
#include <Python.h>
#include <iostream>
#include <vector>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace hukou {
    std::string query_admin_name(std::string_view code, int birth_year);
    bool isValidDate(int year, int month, int day);
    bool checkIDCard(std::string_view id);
    bool matchID(std::string_view id);
    bool init_python();
    void finalize_python();
}