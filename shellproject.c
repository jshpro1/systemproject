#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_INPUT_LENGTH 256
#define MAX_ARG_COUNT 50

int getargs(char *cmd, char **argv);

void handle_signal(int signo) { 
	/* 시그널 핸들러 */
	if (signo == SIGINT) {
		printf("\nCaught Ctrl-C (SIGINT)\n");
	} else if (signo == SIGQUIT) {
		printf("\nCaught Ctrl-Z (SIGQUIT)\n");
	}
}

/* 내부 명령어 구현 */
void my_ls() {
	/* 기본 ls 구현 */
	execlp("ls", "ls", NULL);
}

void my_pwd() {
	/* 기본 pwd 구현 */
	execlp("pwd", "pwd", NULL);
}

void my_cd(char *dir) {
	/* 기본 cd 구현 */
	if (dir == NULL) {
		fprintf(stderr, "Usage: cd <directory>\n");
	} else {
		if (chdir(dir) != 0) {
			perror("cd 실패");
		}
	}
}

void my_mkdir(char *dir) {
	/* 기본 mkdir 구현 */
	if (dir == NULL) {
		fprintf(stderr, "Usage: mkdir <directory>\n");
	} else {
		if (mkdir(dir, 0777) != 0) {
			perror("mkdir 실패");
		}
	}
}

void my_rmdir(char *dir) {
	/* 기본 rmdir 구현 */
	if (dir == NULL) {
		fprintf(stderr, "Usage: rmdir <directory>\n");
	} else {
		if (rmdir(dir) != 0) {
			perror("rmdir 실패");
		}
	}
}

void my_ln(char *source, char *link_name) {
	/* 기본 ln 구현 */
	if (source == NULL || link_name == NULL) {
		fprintf(stderr, "Usage: ln <source> <link_name>\n");
	} else {
		if (link(source, link_name) != 0) {
			perror("ln 실패");
		}
	}
}

void my_cp(char *source, char *destination) {
	/* 기본 cp 구현 */
	if (source == NULL || destination == NULL) {
		fprintf(stderr, "Usage: cp <source> <destination>\n");
	} else {
		if (execlp("cp", "cp", source, destination, NULL) == -1) {
			perror("cp 실패");
		}
	}
}

void my_rm(char *file) {
	/* 기본 rm 구현 */
	if (file == NULL) {
		fprintf(stderr, "Usage: rm <file>\n");
	} else {
		if (remove(file) != 0) {
			perror("rm 실패");
		}
	}
}

void my_mv(char *source, char *destination) {
	/* 기본 mv 구현 */
	if (source == NULL || destination == NULL) {
		fprintf(stderr, "Usage: mv <source> <destination>\n");
	} else {
		if (rename(source, destination) != 0) {
			perror("mv 실패");
		}
	}
}

void my_cat(char *file) {
	/* 기본 cat 구현 */
	if (file == NULL) {
		fprintf(stderr, "Usage: cat <file>\n");
	} else {
		execlp("cat", "cat", file, NULL);
	}
}

int main() {
	char buf[MAX_INPUT_LENGTH];
	char *argv[MAX_ARG_COUNT];
	int narg;
	pid_t pid;
	int background = 0;

	/* 시그널 핸들러 등록 */
	signal(SIGINT, handle_signal);
	signal(SIGQUIT, handle_signal);

	while (1) {
		printf("shell> ");
		if (fgets(buf, sizeof(buf), stdin) == NULL) {
			perror("fgets 실패");
			exit(EXIT_FAILURE);
		}

		/* 줄 바꿈 문자를 제거합니다. */
		buf[strcspn(buf, "\n")] = '\0';

		/* "exit" 입력 시 프로그램 종료 */
		if (strcmp(buf, "exit") == 0) {
			printf("프로그램을 종료합니다.\n");
			exit(EXIT_SUCCESS);
		}

		/* 백그라운드 실행 여부 확인 */
		if (buf[strlen(buf) - 1] == '&') {
			background = 1;
			buf[strlen(buf) - 1] = '\0'; /* '&' 제거 */
		} else {
			background = 0;
		}

		/* 내부 명령어 처리 */
		if (strncmp(buf, "ls", 2) == 0) {
			my_ls();
		} else if (strncmp(buf, "pwd", 3) == 0) {
			my_pwd();
		} else if (strncmp(buf, "cd", 2) == 0) {
			char *dir = strtok(buf + 2, " ");
			my_cd(dir);
		} else if (strncmp(buf, "mkdir", 5) == 0) {
			char *dir = strtok(buf + 5, " ");
			my_mkdir(dir);
		} else if (strncmp(buf, "rmdir", 5) == 0) {
			char *dir = strtok(buf + 5, " ");
			my_rmdir(dir);
		} else if (strncmp(buf, "ln", 2) == 0) {
			char *source = strtok(buf + 2, " ");
			char *link_name = strtok(NULL, " ");
			my_ln(source, link_name);
		} else if (strncmp(buf, "cp", 2) == 0) {
			char *source = strtok(buf + 2, " ");
			char *destination = strtok(NULL, " ");
			my_cp(source, destination);
		} else if (strncmp(buf, "rm", 2) == 0) {
			char *file = strtok(buf + 2, " ");
			my_rm(file);
		} else if (strncmp(buf, "mv", 2) == 0) {
			char *source = strtok(buf + 2, " ");
			char *destination = strtok(NULL, " ");
			my_mv(source, destination);
		} else if (strncmp(buf, "cat", 3) == 0) {
			char *file = strtok(buf + 3, " ");
			my_cat(file);
		} else {
			
		/* 외부 명령어 실행 */
		narg = getargs(buf, argv);

		pid = fork();	
		if (pid == 0) {
			/* 자식 프로세스 */
			/* 파일 재지향 및 파이프 기능 추가 */
			int input_fd, output_fd;
			char *input_file, *output_file;
		
		if ((input_file = strchr(buf, '<')) != NULL) {
			*input_file = '\0';
			input_file = strtok(input_file + 1, " ");
			input_fd = open(input_file, O_RDONLY);
			dup2(input_fd, 0);
			close(input_fd);
		}
		if ((output_file = strchr(buf, '>')) != NULL) {
			*output_file = '\0';
			output_file = strtok(output_file + 1, " ");
			output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			dup2(output_fd, 1);
			close(output_fd);
		}

		execvp(argv[0], argv);
		perror("execvp 실패");
		exit(EXIT_FAILURE);
		} else if (pid > 0) {
			/* 부모 프로세스 */
			if (!background) {
				/* 백그라운드 실행이 아닌 경우 대기 */
				waitpid(pid, NULL, 0);
			}
		} else {
			perror("fork 실패");
			exit(EXIT_FAILURE);
		}
	}
}

	return 0;
}

int getargs(char *cmd, char **argv) {
	int narg = 0;
	while (*cmd) {
		if (*cmd == ' ' || *cmd == '\t')
			*cmd++ = '\0';
		else {
			argv[narg++] = cmd++;
			while (*cmd != '\0' && *cmd != ' ' && *cmd != '\t')
				cmd++;
		}
	}
	argv[narg] = NULL;
	return narg;
}
