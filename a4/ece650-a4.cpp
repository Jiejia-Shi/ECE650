// defined std::unique_ptr
#include <memory>
// defines Var and Lit
#include "minisat/core/SolverTypes.h"
// defines Solver
#include "minisat/core/Solver.h"

#include <sstream>
#include <vector>
#include <string>
#include <regex>
#include <queue>
#include <stack>

// defined std::cout
#include <iostream>


void printGraph (std::vector<std::vector<int>>& graph) {
	for (const auto& row : graph) { 
        for (int val : row) { 
            std::cout << val << " ";
        }
        std::cout << std::endl; 
    }
}


bool buildGraph(std::string s, std::vector<std::vector<int>>& graph, int vertexNumber) {
	//initialize the graph
	int n = vertexNumber + 1;
	graph.resize(n, std::vector<int>(n, 0));
	
	// use regress expression
	std::regex re("<(\\d+),(\\d+)>");
    std::smatch match;
    std::sregex_iterator iter(s.begin(), s.end(), re);
    std::sregex_iterator end;
    
    // use every point to build the graph
    while (iter != end) {
        // find a point
        match = *iter;
        int a = std::stoi(match[1].str());
        int b = std::stoi(match[2].str());
        
        // check if a or b is incorrect
        // 1. larger than vertexNumber or less than 1
        // 2. self cycle
        // 3. edge already exists
        if (a > vertexNumber || a < 1 || b > vertexNumber || b < 1 || a == b) {
        	std::cerr << "Error: incorrect edge with vertex a or b \n";
        	return false;
		}
		if (graph[a][b] == 1 && graph[b][a] == 1) {
			std::cerr << "Error: this edge already exists \n";
			return false;
		}
		
		// build the graph
		graph[a][b] = 1;
		graph[b][a] = 1;
		
		++iter;
    }
    
    // printGraph(graph);
    return true;
}


int main(int argc, char** argv) {
	
	std::string line;
    
    //graph information
    int vertexNumber = 0;
    std::vector<std::vector<int>> graph;

    // read from stdin until EOF
    while (std::getline(std::cin, line)) {
    	// std::cout << line << std::endl;
    	// std::cout << "Processing line: " << line << std::endl;

        // create an input stream based on the line
        // we will use the input stream to parse the line
        std::istringstream input(line);

        // we expect each line to contain a list of numbers
        // this vector will store the numbers.
        // they are assumed to be unsigned (i.e., positive)
        std::vector<unsigned> nums;

        // while there are characters in the input line
        // unsigned num;
        // parse an integer
        char c;
        if (!(input >> c)) {
            std::cerr << "Error: parsing a char \n";
            continue;
        }

		// information of vertex
		if (c == 'V') {
			// read the vertex number
			int number;
			input >> number;
			if (input.fail()) {
            	std::cerr << "Error: parsing a vertex number \n";
            	continue;
        	}
        	vertexNumber = number;
        	if (vertexNumber < 2) {
        		std::cerr << "Error: vertex number can not be less than 2 \n";
        		continue;
			}
		}
		// information of edge
		else if (c == 'E') {
			std::string s;
			input >> s;
			if (input.fail()) {
            	std::cerr << "Error: parsing a vertex number \n";
            	continue;
        	}
        	if (s == "{}") {
				std::cout << "" << std::endl;
				return 0;
			}
        	bool success = buildGraph(s, graph, vertexNumber);
        	if (!success) {
        		continue;
			}
			break;
		}
		else {
			std::cerr << "Error: not a valid command \n";
			continue;
		}

    }
	
	// calculate answer using SAT
	for (int k = 1; k <= vertexNumber; k++) {
		// initialize all the literals
		int n = vertexNumber;
		std::vector<std::vector<Minisat::Lit>> x(n, std::vector<Minisat::Lit>(k));
		std::unique_ptr<Minisat::Solver> solver(new Minisat::Solver());
		
		// start from 0 !!
		for (int i = 0; i < n; i++) {
    		for (int j = 0; j < k; j++) {
        		x[i][j] = Minisat::mkLit(solver->newVar());
    		}
		}
		
		// At least one vertex is the ith vertex in the vertex cover
		// x(1,i) V x(2,i) V ... V x(n,i) (i from 1 to k)
		for (int i = 0; i < k; i++) {
			Minisat::vec<Minisat::Lit> clause1;
			for (int j = 0; j < n; j++) {
				clause1.push(x[j][i]);
			}
			solver->addClause(clause1);
		}
		
		// No one vertex can appear twice in a vertex cover
		// m:1-n p,q 1-k  when p<q, !x(m,p) V !x(m,q)
		for (int m = 0; m < n; m++) {
			for (int p = 0; p < k - 1; p++) {
				for (int q = p + 1; q < k; q++) {
					Minisat::vec<Minisat::Lit> clause2;
					clause2.push(~x[m][p]);
					clause2.push(~x[m][q]);
					solver->addClause(clause2);
				}
			}
		}
		
		// No more than one vertex appears in the mth position of the vertex cover
		// m:1-k p,q 1-n  when p<q, !x(p,m) V !x(q,m)
		for (int m = 0; m < k; m++) {
			for (int p = 0; p < n - 1; p++) {
				for (int q = p + 1; q < n; q++) {
					Minisat::vec<Minisat::Lit> clause3;
					clause3.push(~x[p][m]);
					clause3.push(~x[q][m]);
					solver->addClause(clause3);
				}
			}
		}
		
		// Every edge is incident to at least one vertex in the vertex cover
		// i,j in E, x(i,1) V x(i,2) ... V x(i,k) V x(j,1) V x(j,2) ... V x(j,k)
		for (int i = 1; i <= n; i++) {
			for (int j = i + 1; j <= n; j++) {
				if (graph[i][j] == 1) {
					Minisat::vec<Minisat::Lit> clause4;
					for (int a = 0; a < k; a++) {
						clause4.push(x[i - 1][a]);
						clause4.push(x[j - 1][a]);
					}
					solver->addClause(clause4);
				}
			}
		}
		
		bool res = solver->solve();
		
		if (res) {
            
            std::string cover = "";
            
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < k; j++) {
                    Minisat::lbool value = solver->modelValue(x[i][j]);
                    bool isTrue = (Minisat::toInt(value) == 0);
                    if (isTrue) {
                    	cover = cover + std::to_string(i + 1);
                    	cover = cover + " ";
					}
                }
            }
            // remove excess space
			if (!cover.empty()) {
        		cover.pop_back();
    		}
    		
            std::cout << cover << std::endl;
            
            break; // Found a solution, exit loop
        } 

	}
	
}

