#include "OMPClauses.h"

#include "clang/AST/Decl.h"

#include <assert.h>
#include <sstream>
#include <iostream>

extern std::vector<clang::ValueDecl *> globals;

OMPClauses::OMPClauses() {
}

OMPClauses::OMPClauses(std::string clauses) {
    std::vector<std::string> split_clauses;
    std::stringstream acc;
    int paren_depth = 0;
    int brace_depth = 0;
    int start = 0;
    int index = 0;

    while (index < clauses.size()) {
        if (clauses[index] == ' ' && paren_depth == 0 && brace_depth == 0) {
            int seek = index;
            while (seek < clauses.size() && clauses[seek] == ' ') seek++;
            if (seek < clauses.size() && clauses[seek] == '(') {
                index = seek;
            } else {
                split_clauses.push_back(clauses.substr(start,
                            index - start));
                index++;
                while (index < clauses.size() && clauses[index] == ' ') {
                    index++;
                }
                start = index;
            }
        } else {
            if (clauses[index] == '(') paren_depth++;
            else if (clauses[index] == ')') paren_depth--;
            else if (clauses[index] == '[') brace_depth++;
            else if (clauses[index] == ']') brace_depth--;
            index++;
        }
    }
    if (index != start) {
        split_clauses.push_back(clauses.substr(start));
    }

    for (std::vector<std::string>::iterator i = split_clauses.begin(),
            e = split_clauses.end(); i != e; i++) {
        std::string clause = *i;

        std::string clauseName;
        SingleClauseArgs *clauseArgs = NULL;
        if (clause.find("(") == std::string::npos) {
            clauseName = clause;
            clauseArgs = new SingleClauseArgs(clauseName);
        } else {
            size_t end = clause.find("(");
            clauseName = clause.substr(0, end);
            while (clauseName.at(clauseName.size() - 1) == ' ') {
                clauseName = clauseName.substr(0, clauseName.size() - 1);
            }
            std::string withoutOpenParen = clause.substr(end + 1);
            assert(withoutOpenParen[withoutOpenParen.size() - 1] == ')');
            std::string withoutParens = withoutOpenParen.substr(0,
                    withoutOpenParen.size() - 1);
            clauseArgs = new SingleClauseArgs(clauseName, withoutParens);
        }

        if (parsedClauses.find(clauseName) == parsedClauses.end()) {
            parsedClauses.insert(
                    std::pair<std::string, std::vector<SingleClauseArgs *> *>(
                        clauseName, new std::vector<SingleClauseArgs *>()));
        }
        parsedClauses.at(clauseName)->push_back(clauseArgs);
    }
}

bool OMPClauses::hasClause(std::string clause) {
    return parsedClauses.find(clause) != parsedClauses.end();
}

std::string OMPClauses::getSingleArg(std::string clause) {
    assert(hasClause(clause));
    assert(parsedClauses.at(clause)->size() == 1);
    SingleClauseArgs *arg = parsedClauses.at(clause)->at(0);
    return arg->getSingleArg();
}

std::vector<SingleClauseArgs *> *OMPClauses::getArgs(std::string clause) {
    assert(hasClause(clause));
    return parsedClauses.at(clause);
}

std::map<std::string, std::vector<SingleClauseArgs *> *>::iterator
OMPClauses::begin() {
    return parsedClauses.begin();
}

std::map<std::string, std::vector<SingleClauseArgs *> *>::iterator
OMPClauses::end() {
    return parsedClauses.end();
}

std::vector<std::string> *OMPClauses::getFlattenedArgsList(std::string clause) {
    std::vector<std::string> *flattened = new std::vector<std::string>();

    std::vector<SingleClauseArgs *> *allArgs = getArgs(clause);
    for (std::vector<SingleClauseArgs *>::iterator i = allArgs->begin(),
            e = allArgs->end(); i != e; i++) {
        std::vector<std::string> *singleClause = (*i)->getArgs();
        for (std::vector<std::string>::iterator ii = singleClause->begin(),
                ee = singleClause->end(); ii != ee; ii++) {
            flattened->push_back(*ii);
        }
    }
    return flattened;
}

int OMPClauses::getNumCollapsedLoops() {
    assert(hasClause("for"));
    if (hasClause("collapse")) {
        std::string collapseArg = getSingleArg("collapse");
        return atoi(collapseArg.c_str());
    } else {
        return 1;
    }
}

void OMPClauses::addClauseArg(std::string clause, std::string arg) {
    if (!hasClause(clause)) {
        parsedClauses.insert(
                std::pair<std::string, std::vector<SingleClauseArgs *> *>(
                    clause, new std::vector<SingleClauseArgs *>()));
    }
    parsedClauses.at(clause)->push_back(new SingleClauseArgs(clause, arg));
}
