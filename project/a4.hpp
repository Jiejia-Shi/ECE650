#pragma once
#include <string>
#include <vector>

void VertexSpec(const std::string &line, int &num, std::vector<std::vector<int>> &adjList);
void EdgeSpec(const std::string &line, int num, std::vector<std::vector<int>> &adjList, std::set<std::pair<int, int>> &edgeSet);
bool VertexCover(int num, std::vector<std::vector<int>> &adjList);
void ApproxVC1(int num, std::vector<std::vector<int>> &adjList);
void ApproxVC2 (const std::set<std::pair<int, int>> edgeSet);
