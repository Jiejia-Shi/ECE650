#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <set>
#include <memory>
#include <chrono>
#include "minisat/core/SolverTypes.h"
#include "minisat/core/Solver.h"
#include <algorithm>
#include <functional>

bool error = false;

void VertexSpec(const std::string &line, int &num, std::vector<std::vector<int>> &adjList)
{
    if (error)
        return;
    std::istringstream iss(line);
    char c;
    int n;
    if (iss >> c >> n)
    {
        num = n;
        // std::cout << "input number:" << n << std::endl;
        if (n < 2)
        {
            std::cerr << "Error: Invalid Vertex " << num;
            error = true;
            return;
        }
        adjList.clear();
        adjList.resize(num + 1);
    }
}
void EdgeSpec(const std::string &line, int num, std::vector<std::vector<int>> &adjList, std::set<std::pair<int, int>> &edgeSet)
{
    if (error)
        return;
    std::istringstream iss(line);
    char c;
    int x, y;
    // skip E,{ and wp
    iss >> c >> c;
    while (iss >> c && c == '<')
    {
        iss >> x >> c >> y;
        if (x < 1 || x > num || y < 1 || y > num || x == y)
        {
            std::cerr << "Error: Invalid Edge <" << x << "," << y << ">";
            adjList.clear();
            adjList.resize(num + 1);
            error = true;
            return;
        }
        int min = std::min(x, y);
        int max = std::max(x, y);
        if (edgeSet.find({min, max}) != edgeSet.end())
        {
            std::cerr << "Error: Invalid Edge: <" << x << "," << y << ">";
            adjList.clear();
            adjList.resize(num + 1);
            error = true;
            return;
        }
        adjList[max].push_back(min);
        adjList[min].push_back(max);
        edgeSet.insert({min, max});
        iss >> c >> c;
    }
}

// A basic building block for sorting networks that ensures correct ordering of two input literals.
void addComparator(Minisat::Solver &solver, Minisat::Lit in1, Minisat::Lit in2, Minisat::Lit &out_max, Minisat::Lit &out_min)
{
    out_max = Minisat::mkLit(solver.newVar());
    out_min = Minisat::mkLit(solver.newVar());

    // out_max = in1 OR in2
    solver.addClause(~in1, out_max);
    solver.addClause(~in2, out_max);
    solver.addClause(in1, in2, ~out_max);

    // out_min = in1 AND in2
    solver.addClause(in1, ~out_min);
    solver.addClause(in2, ~out_min);
    solver.addClause(~in1, ~in2, out_min);
}

void mergeNetworks(Minisat::Solver &solver, const std::vector<Minisat::Lit> &left, const std::vector<Minisat::Lit> &right, std::vector<Minisat::Lit> &outputs)
{
    size_t n = left.size() + right.size();

    if (n == 2)
    {
        // Base case: compare the two elements
        Minisat::Lit out_max, out_min;
        addComparator(solver, left[0], right[0], out_max, out_min);
        outputs[0] = out_min;
        outputs[1] = out_max;
        return;
    }

    // Split into even and odd indexed elements
    std::vector<Minisat::Lit> leftEven, leftOdd, rightEven, rightOdd;
    for (size_t i = 0; i < left.size(); ++i)
    {
        if (i % 2 == 0)
            leftEven.push_back(left[i]);
        else
            leftOdd.push_back(left[i]);
    }
    for (size_t i = 0; i < right.size(); ++i)
    {
        if (i % 2 == 0)
            rightEven.push_back(right[i]);
        else
            rightOdd.push_back(right[i]);
    }

    // Recursively merge even and odd indexed elements
    std::vector<Minisat::Lit> evenOutputs, oddOutputs;
    mergeNetworks(solver, leftEven, rightEven, evenOutputs);
    mergeNetworks(solver, leftOdd, rightOdd, oddOutputs);

    // Combine even and odd outputs
    outputs.resize(n);
    size_t i = 0;
    for (; i < evenOutputs.size(); ++i)
        outputs[2 * i] = evenOutputs[i];
    for (i = 0; i < oddOutputs.size(); ++i)
        outputs[2 * i + 1] = oddOutputs[i];

    // Add comparators between adjacent outputs
    for (size_t i = 1; i < n - 1; i += 2)
    {
        Minisat::Lit out_max, out_min;
        addComparator(solver, outputs[i], outputs[i + 1], out_max, out_min);
        outputs[i] = out_min;
        outputs[i + 1] = out_max;
    }
}

// Sorts a list of literals based on their truth values, producing an ordered output.
void buildSortingNetwork(Minisat::Solver &solver, const std::vector<Minisat::Lit> &inputs, std::vector<Minisat::Lit> &outputs)
{
    size_t n = inputs.size();

    if (n == 1)
    {
        outputs = inputs;
        return;
    }

    // Split the inputs into two halves
    size_t mid = n / 2;
    std::vector<Minisat::Lit> left(inputs.begin(), inputs.begin() + mid);
    std::vector<Minisat::Lit> right(inputs.begin() + mid, inputs.end());

    // Recursively sort both halves
    std::vector<Minisat::Lit> sortedLeft, sortedRight;
    buildSortingNetwork(solver, left, sortedLeft);
    buildSortingNetwork(solver, right, sortedRight);

    // Merge the sorted halves
    outputs.resize(n);
    mergeNetworks(solver, sortedLeft, sortedRight, outputs);
}

void addAtMostKConstraint(Minisat::Solver &solver, const std::vector<Minisat::Lit> &literals, int k)
{
    int n = literals.size();
    if (n <= k)
        return; // The constraint is always satisfied

    std::vector<int> indices(k + 1);

    // Define the lambda with std::function to enable recursion
    std::function<void(int, int)> generate = [&](int start, int depth)
    {
        if (depth == k + 1)
        {
            Minisat::vec<Minisat::Lit> clause;
            for (int idx : indices)
            {
                clause.push(~literals[idx]);
            }
            solver.addClause(clause);
            return;
        }
        for (int i = start; i <= n - (k + 1 - depth); ++i)
        {
            indices[depth] = i;
            generate(i + 1, depth + 1); // Recursive call
        }
    };

    // Start the recursive process
    generate(0, 0);
}

bool VertexCover(int num, std::vector<std::vector<int>> &adjList)
{
    // timeout
    int timeout = 10;
    // start time
    auto start = std::chrono::high_resolution_clock::now();
    for (int k = 1; k <= num; ++k)
    {
        // every k end time
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        // timeout
        if (elapsed.count() >= timeout)
        {
            std::cout << "CNF-SAT-VC: timeout" << std::endl;
            break;
        }

        std::unique_ptr<Minisat::Solver> solver(new Minisat::Solver());
        std::vector<Minisat::Lit> vertexLits(num + 1);

        // Create input variables for each vertex
        for (int i = 1; i <= num; i++)
        {
            vertexLits[i] = Minisat::mkLit(solver->newVar());
        }

        // Add cardinality constraint: At most k vertices in the cover
        std::vector<Minisat::Lit> selectedVertices(vertexLits.begin() + 1, vertexLits.end());
        addAtMostKConstraint(*solver, selectedVertices, k);

        // Add edge coverage constraints
        for (int i = 1; i <= num; i++)
        {
            for (int neighbor : adjList[i])
            {
                if (neighbor > i)
                {
                    solver->addClause(vertexLits[i], vertexLits[neighbor]);
                }
            }
        }

        // Solve the SAT problem
        bool res = solver->solve();
        if (res)
        {
            std::set<int> uniqueCover;
            for (int i = 1; i <= num; i++)
            {
                if (solver->modelValue(vertexLits[i]) == Minisat::l_True)
                {
                    uniqueCover.insert(i); // Avoid duplicates
                }
            }

            // Convert the set to a vector and sort it
            std::vector<int> vertexCover(uniqueCover.begin(), uniqueCover.end());
            std::sort(vertexCover.begin(), vertexCover.end());

            // Output the vertex cover with prefix
            std::string ans = "CNF-SAT-VC: ";
            // Output the vertices in the vertex cover
            for (size_t l = 0; l < vertexCover.size(); l++)
            {
                ans = ans + std::to_string(vertexCover[l]);
                if (l < vertexCover.size() - 1)
                    ans = ans + ",";
            }
            std::cout << ans << std::endl;

            return true;
        }
    }
    return false;
}

void ApproxVC1(int num, std::vector<std::vector<int>> &inial_adjList)
{
    std::vector<int> vertexCover;
    std::vector<bool> included(num + 1, false);
    std::vector<std::vector<int>> adjList = inial_adjList;

    while (true)
    {
        int maxDegree = 0;
        int vertex = -1;

        for (int i = 1; i <= num; ++i)
        {
            if (included[i])
                continue;

            int degree = adjList[i].size();
            if (degree > maxDegree)
            {
                maxDegree = degree;
                vertex = i;
            }
        }

        if (vertex == -1)
            break;

        vertexCover.push_back(vertex);
        included[vertex] = true;

        for (int neighbor : adjList[vertex])
        {
            adjList[neighbor].erase(
                std::remove(adjList[neighbor].begin(), adjList[neighbor].end(), vertex),
                adjList[neighbor].end());
        }
        adjList[vertex].clear();
    }

    std::sort(vertexCover.begin(), vertexCover.end());
    std::string ans = "APPROX-VC-1: ";
    for (size_t i = 0; i < vertexCover.size(); ++i)
    {
        ans = ans + std::to_string(vertexCover[i]);
        if (i < vertexCover.size() - 1)
        {
            ans = ans + ",";
        }
    }
    std::cout << ans << std::endl;
}
void ApproxVC2(const std::set<std::pair<int, int>> edgeSet)
{
    std::set<int> result;
    // find result
    for (const auto &edge : edgeSet)
    {
        int vertex1 = edge.first;
        int vertex2 = edge.second;
        if (result.count(vertex1) == 0 && result.count(vertex2) == 0)
        {
            result.insert(vertex1);
            result.insert(vertex2);
        }
    }
    // build and output result string
    std::string res = "APPROX-VC-2: ";
    for (const auto &vertex : result)
    {
        res += std::to_string(vertex);
        res.append(",");
    }
    res.resize(res.size() - 1);
    std::cout << res << std::endl;
}
