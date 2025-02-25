#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <algorithm>
#include <signal.h>
#include <sys/types.h>
#include <regex>

pid_t pid1, pid2, pid3;


void terminate_all_processes() {
	kill(pid1, SIGTERM);
	kill(pid2, SIGTERM);
	kill(pid3, SIGTERM);
}

void sigintHandler(int signum) {

    terminate_all_processes();

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    exit(0);  // Exit the parent process
}

bool isInteger(const std::string& str) {
    std::regex integerRegex("^-?\\d+$");
    return std::regex_match(str, integerRegex);
}

int main (int argc, char **argv) {
	
	signal(SIGINT, sigintHandler);
	
	int opt;
	int s = 10, n = 5, l = 5, c = 20;
	
	// accept command arguments
	while ((opt = getopt(argc, argv, "s:n:l:c:")) != -1) {
		switch (opt) {
			case 's':
				if (!isInteger(optarg)) {
                    std::cerr << "Error: s is not an integer\n";
                    return 1;
                }
				s = std::stoi(optarg);
				if (s < 2) {
					std::cerr << "Error: s should >= 2\n";
					return 1;
				}
				break;
				//todo: what if s < 2 
			case 'n':
				if (!isInteger(optarg)) {
                    std::cerr << "Error: n is not an integer\n";
                    return 1;
                }
				n = std::stoi(optarg);
				if (n < 1) {
					std::cerr << "Error: n should >= 1\n";
                    return 1;
				}
				break;
			case 'l':
				if (!isInteger(optarg)) {
                    std::cerr << "Error: l is not an integer\n";
                    return 1;
                }
				l = std::stoi(optarg);
				if (l < 5) {
					std::cerr << "Error: l should >= 5\n";
                    return 1;
				}
				break;
			case 'c':
				if (!isInteger(optarg)) {
                    std::cerr << "Error: c is not an integer\n";
                    return 1;
                }
				c = std::stoi(optarg);
				if (c < 1) {
					std::cerr << "Error: c should >= 1\n";
                    return 1;
				}
				break;
			default:
				std::cerr << "Error: unexpected command argument";
				return 1;
		}
	}
	
	// create pipes
	int pipe1[2], pipe2[2], pipe3[2];
	
	if (pipe(pipe1) == -1 || pipe(pipe2) == -1 || pipe(pipe3) == -1) {
        perror("pipe creation failed");
        exit(1);
    }
	
	// 1. rgen program
	pid1 = fork();
	if (pid1 == 0) {
		setvbuf(stderr, NULL, _IONBF, 0);
		
		// close read
		close(pipe1[0]);
		// redirect write
		dup2(pipe1[1], STDOUT_FILENO);
		close(pipe1[1]);
		execl("./rgen", "rgen", "-s", std::to_string(s).c_str(), "-n", std::to_string(n).c_str(), "-l", std::to_string(l).c_str(), "-c", std::to_string(c).c_str(), (char *)NULL);
		perror("Error: execl 1 failed");
		exit(1);
	}
	
	// 2. a1 program (generate graph) (python)
	pid2 = fork();
	if (pid2 == 0) {
		close(pipe1[1]);
		dup2(pipe1[0],STDIN_FILENO);
		close(pipe1[0]);
		
		close(pipe2[0]);
		dup2(pipe2[1], STDOUT_FILENO);
		close(pipe2[1]);
		execl("/usr/bin/python3", "python3", "./ece650-a1.py", (char *)NULL);
		perror("Error: execl 2 failed");
		exit(1);
	}
	
	// 3. a2 program (find path)
	pid3 = fork();
	if (pid3 == 0) {
		close(pipe3[1]);
		dup2(pipe3[0],STDIN_FILENO);
		close(pipe3[0]);

		execl("./ece650-a2", "ece650-a2", (char *)NULL);
		perror("Error: execl 3 failed");
		exit(1);
	}

	// pid4 the parent pid, accept output from a1 and terminal, pass them to a2
	close(pipe2[1]);
	close(pipe3[0]);
	
	fd_set read_fds;
	int max_fd = std::max(pipe2[0], STDIN_FILENO) + 1;
	char buffer[100];
	
	while (true) {
		//check if any child process has exited and then kill all
		int status;
        pid_t exited_pid = waitpid(-1, &status, WNOHANG);
        if (exited_pid == pid1 || exited_pid == pid2 || exited_pid == pid3) {
            terminate_all_processes();
            break;
        }
		
		//accept output from a1 and terminal, pass them to a2
		FD_ZERO(&read_fds);
        FD_SET(pipe2[0], &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        
        int activity = select(max_fd, &read_fds, NULL, NULL, NULL);
    	if (activity == -1) {
        	std::cerr << "Select failed!" << std::endl;
        	break;
    	}
    
		if (FD_ISSET(pipe2[0], &read_fds)) {
        	ssize_t n = read(pipe2[0], buffer, sizeof(buffer) - 1);
        	if (n > 0) {
            	buffer[n] = '\0';
            	// std::cout << "receive from pid2 " << buffer;
            	// write to pid3 (a2)
            	write(pipe3[1], buffer, strlen(buffer));
        	} else if (n == 0) {
            	break;
        	}
    	}
    	
    	if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
                // std::cout << "receive from terminal " << buffer;
                // write to pid3 (a2)
                write(pipe3[1], buffer, strlen(buffer));
            }
        }
	}
	
	close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe3[1]);

	// wait all pids
	waitpid(pid1, NULL, 0);
	waitpid(pid2, NULL, 0);
	waitpid(pid3, NULL, 0);
	

    return 0;
}
