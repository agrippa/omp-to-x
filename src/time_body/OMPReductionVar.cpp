#include "OMPReductionVar.h"

#include <iostream>

OMPReductionVar::OMPReductionVar(std::string setOp, std::string setVar) :
    op(setOp), var(setVar) {
}

std::string OMPReductionVar::getOp() { return op; }

std::string OMPReductionVar::getVar() { return var; }

std::string OMPReductionVar::getInitialValue() {
    if (op == "+") {
        return std::string("0");
    } else {
        std::cerr << "Unsupported reduction op \"" << op << "\"" << std::endl;
        exit(1);
    }
}
