//Hukou_helper.cpp
#include"Hukou_Helper.h"


static PyObject* pModule = nullptr;
static PyObject* pFuncQuery = nullptr;

bool hukou::init_python() {
    if (!Py_IsInitialized()) {
        Py_Initialize();
    }

    PyRun_SimpleString("import sys; sys.path.append('.')");

    PyObject* pName = PyUnicode_FromString("fetch_data");
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (!pModule) {
        PyErr_Print();
        std::cerr << "Failed to load fetch_data module.\n";
        return false;
    }

    pFuncQuery = PyObject_GetAttrString(pModule, "query_admin_name");
    if (!pFuncQuery || !PyCallable_Check(pFuncQuery)) {
        PyErr_Print();
        std::cerr << "Cannot find callable query_admin_name.\n";
        Py_XDECREF(pFuncQuery);
        Py_DECREF(pModule);
        pFuncQuery = nullptr;
        pModule = nullptr;
        return false;
    }
    return true;
}

void hukou::finalize_python() {
    Py_XDECREF(pFuncQuery);
    Py_XDECREF(pModule);
    if (Py_IsInitialized()) {
        Py_Finalize();
    }
}

std::string hukou::query_admin_name(std::string_view code, int birth_year) {
    if (!pFuncQuery) return {};

    PyObject* pArgs = PyTuple_Pack(2,
        PyUnicode_FromStringAndSize(code.data(), code.size()),
        PyLong_FromLong(birth_year));
    PyObject* pValue = PyObject_CallObject(pFuncQuery, pArgs);
    Py_DECREF(pArgs);

    if (!pValue) {
        PyErr_Print();
        return {};
    }

    std::string result;
    if (PyUnicode_Check(pValue)) {
        const char* str = PyUnicode_AsUTF8(pValue);
        if (str) result.assign(str);
    }
    Py_DECREF(pValue);
    return result;
}

bool hukou::isValidDate(int year, int month, int day) {
    if (year < 1900 || year > 2100) return false;
    if (month < 1 || month > 12) return false;

    constexpr static int days_in_month[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    int max_day = days_in_month[month - 1];

    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
        max_day = 29;
    }
    return day >= 1 && day <= max_day;
}

inline int fast_atoi(std::string_view s) {
    int value = 0;
    for (char c : s) value = value * 10 + (c - '0');
    return value;
}

bool hukou::checkIDCard(std::string_view id) {
    if (id.size() != 18) return false;

    for (int i = 0; i < 17; ++i) {
        if (!std::isdigit(static_cast<unsigned char>(id[i]))) return false;
    }
    char last = id[17];
    if (!(std::isdigit(static_cast<unsigned char>(last)) || last == 'X' || last == 'x')) return false;

    int year = fast_atoi(id.substr(6, 4));
    int month = fast_atoi(id.substr(10, 2));
    int day = fast_atoi(id.substr(12, 2));
    if (!isValidDate(year, month, day)) return false;

    constexpr static int weight[17] = { 7, 9, 10, 5, 8, 4, 2, 1, 6, 3, 7, 9, 10, 5, 8, 4, 2 };
    constexpr static char check_code[11] = { '1','0','X','9','8','7','6','5','4','3','2' };

    int sum = 0;
    for (int i = 0; i < 17; ++i) {
        sum += (id[i] - '0') * weight[i];
    }
    char expected = check_code[sum % 11];
    return (expected == 'X') ? (last == 'X' || last == 'x') : (last == expected);
}

bool hukou::matchID(std::string_view id) {
    if (!hukou::init_python()) {
        fprintf(stderr, "Python init failed.\n");
        return false;
    }
    if (!hukou::checkIDCard(id)) {
        return false;
    }

    int year = fast_atoi(id.substr(6, 4));
    int month = fast_atoi(id.substr(10, 2));
    int day = fast_atoi(id.substr(12, 2));
    int sex = fast_atoi(id.substr(16, 1)) % 2;

    printf("身份证号合法性校验成功\n");
    printf("出生日期: %d/%d/%d\n", year, month, day);
    printf("性别: %s\n", sex ? "男" : "女");

    auto name = hukou::query_admin_name(id.substr(0, 6), year);
    if (name.empty()) {
        printf("地区：未知\n");
    }
    else {
        printf("地区:");
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
#endif
        printf(name.c_str());
    }

    return !name.empty();
}