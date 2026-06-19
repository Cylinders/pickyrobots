#pragma once
#include <string>

namespace mapf_solvers::cbsh {
    bool cbs_solve(const std::string &mapFilePath, const std::string &scenFilePath, int n);
    bool cbsh_solve(const std::string &mapFilePath, const std::string &scenFilePath, int n);
}
