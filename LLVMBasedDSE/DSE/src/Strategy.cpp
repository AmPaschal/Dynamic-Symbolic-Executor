#include <vector>
#include <algorithm>
#include <fstream>
#include "Strategy.h"
#include "SymbolicInterpreter.h"

/*
 * Implement your search strategy.
 */
void searchStrategy(z3::expr_vector &OldVec) {
    // == doesnt compare. It creates a z3 expression that can be evaluated by the solver.

    // Verify vec is not empty. If empty, return

    if (OldVec.empty()) {
        return;
    }

    static std::vector<std::string> branchesSeen;

    // Get the last item of vec

    z3::expr falseExpr = OldVec.ctx().bool_val(false);

    /*for (int i = 0; i < OldVec.size(); i++) {
        std::cout << "Print here" << "\n";
        if (i == OldVec.size() - 1) {
            z3::expr vector = OldVec[i];
            z3::expr newVector = vector == falseExpr;
            OldVec.set(i, newVector);
        }
    }*/

    // Define branches
    std::vector<std::string> branches;

    // Add all branches to vector
    std::string Line;
    std::ifstream branchFile(BranchFile);
    if (branchFile.is_open()) {
        while (getline(branchFile, Line)) {
            branches.push_back(Line);
        }
    }

    // Loop through branches from the end
    for (int i = branches.size() - 1; i >= 0; i--) {
        std::cout << "Checking i val: " << i << "\n";
        std::string branch = branches.at(i);
        std::cout << "Branch: " << branch << "\n";

        if (std::find(branchesSeen.begin(),  branchesSeen.end(), branch) != branchesSeen.end()) {
            std::cout << "Size of branches seen: " << branchesSeen.size() << "\n";

            if (i == 0) {
                return;
            }
            continue;
        }

        branchesSeen.push_back(branch);
        std::cout << "Size of branches seen now: " << branchesSeen.size() << "\n";

        z3::expr vector = OldVec[i];
        z3::expr newVector = vector == falseExpr;
        OldVec.set(i, newVector);

        break;

    }

    return;

}
