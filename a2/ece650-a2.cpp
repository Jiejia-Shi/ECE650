// Compile with c++ ece650-a2cpp -std=c++11 -o ece650-a2
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <regex>
#include <queue>
#include <stack>

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
    
    return true;
}

std::string findShortestPath(int startVertex, int endVertex, std::vector<std::vector<int>>& graph, int vertexNumber) {
	std::queue<int> q;
	std::vector<int> parent(vertexNumber);
	std::string path = "";
	
	// find the shortest path
	// using BFS 
	q.push(startVertex);
	while (!q.empty()) {
		int vertex = q.front();
		q.pop();
		// push all neighbors in queue and record their parent
		for (int i = 1; i <= vertexNumber; i++) {
			// parent can also be used to check if this vertex has been visited
			// also, startVertex should be excluded
			if (graph[vertex][i] == 1 && parent[i] == 0 && i != startVertex) {
				q.push(i);
				parent[i] = vertex;
				// find endVertex
				if (i == endVertex) {
					// put all vertices in to stack
					std::stack<int> vertices;
					while (endVertex != 0) {
						vertices.push(endVertex);
						endVertex = parent[endVertex];
					}
					// build the path
					while (!vertices.empty()) {
						int v = vertices.top();
						vertices.pop();
						path = path + std::to_string(v);
						if (!vertices.empty()) {
							path = path + "-";
						}
					}
					return path;
				}
			}
		}
	}
	return path;
}

int main(int argc, char** argv) {
    // Test code. Replaced with your code

    // Print command line arguments that were used to start the program
    // std::cout << "Called with " << argc << " arguments\n";
    // for (int i = 0; i < argc; ++i)
    //     std::cout << "Arg " << i << " is " << argv[i] << "\n";
    std::string line;
    
    //graph information
    int vertexNumber = 0;
    std::vector<std::vector<int>> graph;

    // read from stdin until EOF
    while (std::getline(std::cin, line)) {
    	std::cout << line << std::endl;
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
        	bool success = buildGraph(s, graph, vertexNumber);
        	if (!success) {
        		continue;
			}
		}
		// information of shortest path
		else if (c == 's') {
			int startVertex = 0;
			int endVertex = 0;
			input >> startVertex;
			// check start vertex
			if (input.fail() || startVertex > vertexNumber || startVertex < 1) {
				std::cerr << "Error: wrong start vertex \n";
            	continue;
        	}
        	input >> endVertex;
        	// check end vertex
			if (input.fail() || endVertex > vertexNumber || endVertex < 1) {
            	std::cerr << "Error: wrong end vertex \n";
            	continue;
        	}
        	// start vertex and end vertex can not be the same
        	if (startVertex == endVertex) {
        		std::cerr << "Error: start vertex and end vertex are the same \n";
            	continue;
			}
        	// find the shortest path and print
        	std::string path = findShortestPath(startVertex, endVertex, graph, vertexNumber);
        	if (path.empty()) {
        		std::cerr << "Error: the shortest path does not exist \n";
        		continue;
			}
			else {
				std::cout << path << std::endl;
				continue;
			}
		}
		else {
			std::cerr << "Error: not a valid command \n";
			continue;
		}

//        // if eof bail out
//        if (input.eof())
//            break;

    }
}
