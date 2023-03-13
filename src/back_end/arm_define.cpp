#include <arm_define.h>
#include <cassert>

std::string CondToString(Cond cond) {
    switch (cond) {
        case Cond::AL:
        return "";
        case Cond::EQ:
        return "eq";
        case Cond::NE:
        return "ne";
        case Cond::GT:
        return "gt";
        case Cond::GE:
        return "ge";
        case Cond::LT:
        return "lt";
        case Cond::LE:
        return "le";
        default:
        assert(0);
        break;
    }
    return "";
}

Cond GetOppositeCond(Cond cond) {
    switch (cond) {
        case Cond::AL:
        return Cond::AL;
        case Cond::EQ:
        return Cond::NE;
        case Cond::NE:
        return Cond::EQ;
        case Cond::GT:
        return Cond::LE;
        case Cond::GE:
        return Cond::LT;
        case Cond::LT:
        return Cond::GE;
        case Cond::LE:
        return Cond::GT;
        default:
        assert(0);
        break;
    }
    return Cond::AL;
}