#include"Hukou_Helper.h"
using namespace hukou;

int main() {
    std::string id;
    std::getline(std::cin, id);
    matchID(id);
    finalize_python();
    return 0;
}
