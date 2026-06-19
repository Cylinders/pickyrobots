#include <string>
#include <cbsh_adapter.h>
#include <bcp_adapter.h>

bool cbs_solve(const std::string &mapFilePath, const std::string &scenFilePath, const int n) {
    return mapf_solvers::cbsh::cbs_solve(mapFilePath, scenFilePath, n);
}

bool cbsh_solve(const std::string &mapFilePath, const std::string &scenFilePath, const int n) {
    return mapf_solvers::cbsh::cbsh_solve(mapFilePath, scenFilePath, n);
}

std::string bcp_solve(const int t, const std::string& input) {
    return mapf_solvers::bcp::bcp_solve(t, input);
}

// // ALGORITHM 3: MDD SAT
//
// string sat_solve(const string& input) {
//     string s_arg0 = "MyApp";
//     string s_arg1 = "--input-file=" + input;
//     string s_arg2 = "--output-file=my_output.txt";
//
//     char* args[] = { &s_arg0[0], &s_arg1[0], &s_arg2[0] };
//
//     return "Result 4: " + sReloc::MDDSATSolver(3, args);
// }