#include<ir_define.h>
#include<cassert>
#include<memory>
using namespace std;
bool is_relation_oper(IROper oper)
{
    if (oper == IROper::LessI || oper == IROper::LessF ||
        oper == IROper::GreaterI || oper == IROper::GreaterF ||
        oper == IROper::EqualI || oper == IROper::EqualF ||
        oper == IROper::NotEqualI || oper == IROper::NotEqualF ||
        oper == IROper::GreaterEqualI || oper == IROper::GreaterEqualF ||
        oper == IROper::LessEqualI || oper == IROper::LessEqualF) {
        return true;
    }
    return false;
}

IROper opposite_relation_oper(IROper oper)
{
    switch(oper) {
        case IROper::LessI:
        return IROper::GreaterEqualI;
        case IROper::GreaterEqualI:
        return IROper::LessI;
        case IROper::LessF:
        return IROper::GreaterEqualF;
        case IROper::GreaterEqualF:
        return IROper::LessF;
        case IROper::GreaterI:
        return IROper::LessEqualI;
        case IROper::LessEqualI:
        return IROper::GreaterI;
        case IROper::GreaterF:
        return IROper::LessEqualF;
        case IROper::LessEqualF:
        return IROper::GreaterF;
        case IROper::EqualI:
        return IROper::NotEqualI;
        case IROper::NotEqualI:
        return IROper::EqualI;
        case IROper::EqualF:
        return IROper::NotEqualF;
        case IROper::NotEqualF:
        return IROper::EqualF;
    }
}