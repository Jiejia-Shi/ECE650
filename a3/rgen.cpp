#include <iostream>
#include <fstream>
#include <list>
#include <string>
#include <unistd.h>
#include <vector>
#include <regex>

int randomNumber(int a, int b) {
	std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
	
	if (!urandom) {
		std::cerr << "Error: urandom error" << std::endl;
		return -1;
	}
	
	unsigned int value;
	urandom.read(reinterpret_cast<char*>(&value), sizeof(value));
	
	if (!urandom) {
		std::cerr << "Error: urandom error" << std::endl;
		return -1;
	}
	
	urandom.close();
	
	int range = b - a + 1;
	int randomNumber = a + (value % range);
	return randomNumber;
	
}

struct Point {
    int x, y;
};

struct Segment {
	Point A, B;
	
	Segment(Point a, Point b) : A(a), B(b) {}
};

//int orientation(Point p, Point q, Point r) {
//    int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
//    if (val == 0) {
//    	return 0;
//	}
//	else if (val > 0) {
//		return 1;
//	}
//	else {
//		return 2;
//	}
//}
//
//bool onSegment(Point p, Point q, Point r) {
//    return (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) && q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y));
//}
//
////todo: further make sure about the "overlap" rule !!!!!!!
//bool checkOverlap(Point A, Point B, Point C, Point D) {
//	// check if two segments overlap each other
//	
//    int o1 = orientation(A, B, C);
//    int o2 = orientation(A, B, D);
//    int o3 = orientation(C, D, A);
//    int o4 = orientation(C, D, B);
//
//    if (o1 != o2 && o3 != o4){
//    	return true;
//	}
//
//    if (o1 == 0 && onSegment(A, C, B)) return true;
//    if (o2 == 0 && onSegment(A, D, B)) return true;
//    if (o3 == 0 && onSegment(C, A, D)) return true;
//    if (o4 == 0 && onSegment(C, B, D)) return true;
//
//    return false;
//}

bool checkOverlap(Point A, Point B, Point C, Point D) {
	int ax = A.x, ay = A.y, bx = B.x, by = B.y, cx = C.x, cy = C.y, dx = D.x, dy = D.y;
	int minx1 = std::min(ax, bx);
	int maxx1 = std::max(ax, bx);
	int minx2 = std::min(cx, dx);
	int maxx2 = std::max(ax, bx);
	
	int miny1 = std::min(ay, by);
	int maxy1 = std::max(ay, by);
	int miny2 = std::min(cy, dy);
	int maxy2 = std::max(cy, dy);
	
	if (ax == bx || cx == dx) {
		if (ax == bx && cx == dx && ax == cx) {
			if(std::max(miny1, miny2) <= std::min(maxy1, maxy2)) {
				return true;
			}
		}
		return false;
	} 
	
	
	double k1 = static_cast<double>(by - ay) / (bx - ax);
	double b1 = ay - k1 * ax;
	double k2 = static_cast<double>(dy - cy) / (dx - cx);
	double b2 = cy - k2 * cx;
	
	// std::cout << k1 << " " << k2 << " " << b1 << " " << b2 << std::endl;
	
	if (k1 == k2 && b1 == b2) {
		if (ay == by) {
			if (std::max(minx1, minx2) < std::min(maxx1, maxx2)) {
				return true;
			}
		} else {
			if (std::max(miny1, miny2) < std::min(maxy1, maxy2)) {
				return true;
			}
		}
		
	} 
	
	return false;
}



std::string getPoints(int lineSegmentNum, int c, std::vector<Segment>& segmentList, int streetNum, int waitTime) {
	std::string points = "";
	std::vector<Point> pointList;
	
	// generate points
	for (int i = 0; i < lineSegmentNum + 1; i++) {
		if (streetNum == 2 && i == 0) {
			// std::cout << streetNum << " " << i << std::endl;
			// ensure there is at least one intersection
			int segSize = segmentList.size();
			// std::cout << segSize << std::endl;
			int randomIndex = randomNumber(0, segSize - 1);
			Segment segment = segmentList[randomIndex];
			pointList.push_back(segment.A);
			continue;
		}
		int attemptNum = 0;
		int x, y;
		while (true) {
			if (attemptNum == 25) {
				// even if fail 25 times, try again
				std::cerr << "Error: failed to generate valid input for 25 simultaneous attempts\n";
				sleep(waitTime);
				// return "";
				attemptNum = 0;
			}
			x = randomNumber(-c, c);
			y = randomNumber(-c, c);
			Point p = {x, y};
			// test if this point is correct
			int n = pointList.size();
			if (n != 0) {
				Point lastP = pointList[n - 1];
				int xLast = lastP.x;
				int yLast = lastP.y;
				// 1. duplicate point
				if (x == xLast && y == yLast) {
					attemptNum++;
//					std::cout << "duplicate problem" << std::endl;
//					std::cout << x << y << xLast << yLast << std::endl;
					continue;
				}
				// 2. overlap
				// todo: further make sure about the "overlap" rule
				int j = 0;
				int n2 = segmentList.size();
				for (; j < n2; j++) {
					Segment segment = segmentList[j];
					Point A = segment.A;
					Point B = segment.B;
					Point C = lastP;
					Point D = p;
					if (checkOverlap(A, B, C, D)) {
//						std::cout << "overlap problem" << std::endl;
//						std::cout << A.x << " " << A.y << std::endl;
//						std::cout << B.x << " " << B.y << std::endl;
//						std::cout << C.x << " " << C.y << std::endl;
//						std::cout << D.x << " " << D.y << std::endl;
						break;
					}
//					Point p1 = pointList[j];
//					Point p2 = pointList[j + 1];
//					// todo: further make sure 
//					// p2 != lastP is not perfect, we lost one segment, remember this.
//					if (checkOverlap(p1, p2, lastP, p) && (p2.x != lastP.x || p2.y != lastP.y)) {
////						std::cout << "overlap problem" << std::endl;
////						std::cout << p1.x << " " << p1.y << std::endl;
////						std::cout << p2.x << " " << p2.y << std::endl;
////						std::cout << lastP.x << " " << lastP.y << std::endl;
////						std::cout << p.x << " " << p.y << std::endl;
//						break;
//					}
				}
				if (n2 != 0 && j != n2) {
					attemptNum++;
					continue;
				}
				// this point is correct
				pointList.push_back(p);
				segmentList.push_back({lastP, p});
				// std::cout << "push segment" << std::endl;
				break;
			}
			pointList.push_back(p);
			break;
		}
			
	}
	
	// build points string
	for (const Point& p : pointList) {
		std::string pString = "(" + std::to_string(p.x) + "," + std::to_string(p.y) + ")";
		points.append(pString);
	}
	
	return points;
}

int main (int argc, char **argv) {
	
//	if (argc != 5) {
//		std::cerr << "Error: Not enough arguments for rgen program";
//		return 1;
//	}
//	
//	// receive s, n, l, c
//	int s = std::atoi(argv[1]);
//    int n = std::atoi(argv[2]);
//    int l = std::atoi(argv[3]);
//    int c = std::atoi(argv[4]);

	int opt;
	int s = 10, n = 5, l = 5, c = 20;
	
	// accept command arguments
	while ((opt = getopt(argc, argv, "s:n:l:c:")) != -1) {
		switch (opt) {
			case 's':
				s = std::stoi(optarg);
				break;
			case 'n':
				n = std::stoi(optarg);
				break;
			case 'l':
				l = std::stoi(optarg);
				break;
			case 'c':
				c = std::stoi(optarg);
				break;
			default:
				std::cerr << "Error: unexpected command argument";
				return 1;
		}
	}
	
	// std::cout << s << n << l << c;
    
    // street number: random integer in [2,s]
    int streetNum = randomNumber(2, s);
    // line-segments in each street: random integer in [1,n]
    int lineSegmentNum = randomNumber(1, n);
    // wait time: random seconds in [5,l]
    int waitTime = randomNumber(5, l);
    // x and y value: random integers between [-c, c]
    // std::cout << "streetNum = " << streetNum << " lineSeg = " << lineSegmentNum << " waitTime =" << waitTime << std::endl; 
    
    std::list<std::string> streetList;
    std::string command = "";
    std::vector<Segment> segmentList;
    
    while (true) {
    	// 1. rm command
    	if (streetList.size() != 0) {
    		for (std::string street : streetList) {
    			std::string rmString = "rm " + street + "\n";
    			command = command.append(rmString);
			}
			streetList.clear();
			segmentList.clear();
		}
    
    	// 2. add command
    	for (int i = 1; i <= streetNum; i++) {
    		std::string streetName = "\"street" + std::to_string(i) + "\"";
    		streetList.push_back(streetName);
    		
    		// generate points
    		std::string streetPoints = "";
    		streetPoints = getPoints(lineSegmentNum, c, segmentList, i, waitTime);
    		if (streetPoints == "") {
    			// fail 25
    			i--;
    			continue;
			}
    		
    		std::string addString = "add " + streetName + " " + streetPoints + "\n";
    		command = command.append(addString);
		}
		
		// 3. gg command
		command = command.append("gg\n");
    	
    	std::cout << command << std::flush;
    	sleep(waitTime);
    	// clear command
    	command = "";
	}
	// std::cout << "s = " << s << ", n = " << n << ", l = " << l << ", c = " << c << std::endl;
	// std::cout << "streetNum = " << streetNum << " lineSeg = " << lineSegmentNum << " waitTime =" << waitTime << std::endl; 
}

