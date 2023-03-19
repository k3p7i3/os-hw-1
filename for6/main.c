#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

int find_substr(char *str1, char *str2, char *ans) {
    char *pos_pnt = strstr(str1, str2);
    if (pos_pnt == NULL) {
        ans[0] = '-';
        ans[1] = '1';
        return 2;
    }
    int pos = pos_pnt - str1;
    sprintf(ans, "%d", pos);

    int digits = 0;
    while (ans[digits] != 0) {
        ++digits;
    }

    return digits;
}

const int buffer_size = 5001;

int main(int argc, char *argv[]) {
    int fd1[2]; // transfer initial data from 1st process to 2nd
    int fd2[2]; // transfer answer data from 2nd process to 1st

    if (argc < 4) {
        printf("3 arguments needed: input file, output file, substring\n");
        exit(-1);
    }

    if (pipe(fd1) < 0)  {
        printf("Pipe1 open error\n");
        exit(-1);
    }

    if (pipe(fd2) < 0) {
        printf("Pipe2 open error\n");
        exit(-1);
    }

    pid_t chpid1 = fork();
    if (chpid1 == -1) {
        printf("Can't create child process\n");
        exit(-1);
    }
    if (chpid1 > 0) {
        // the "first" process that reads string from input file and writes answer to output file

        char buffer1[buffer_size];
        // close unused channels
        close(fd1[0]);
        close(fd2[1]);

        // read string from input file
        int input_file = open(argv[1], O_RDONLY);

        if (input_file == -1) {
            printf("Can't open input file %s\n", argv[1]);
            exit(-1);
        }

        int str_len = read(input_file, buffer1, buffer_size);
        close(input_file);

        // send 2nd process initial text by writing to fd1[1]
        int written_len = write(fd1[1], buffer1, str_len);
        if (str_len != written_len) {
            printf("Some parts of string were lost while writing!\n");
            exit(-1);
        }
        close(fd1[1]);

        // read answer from 2nd process from fd2[0]
        char answer_pos[5];
        int digits = read(fd2[0], &answer_pos, 5);
        close(fd2[0]);

        // print answer to output file
        int output_file = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0666);
        if (output_file == -1) {
            printf("Can't open output file %s\n", argv[2]);
            exit(-1);               
        }
        int bytes = write(output_file, &answer_pos, digits);
        close(output_file);
        if (bytes != digits) {
            printf("The answer was printed to output with errors\n");
            exit(-1);
        }

    } else if (chpid1 == 0) {
        // the 'second' (first-child) process that finds substring
        char buffer2[buffer_size];

        // close unused channels;
        close(fd1[1]);
        close(fd2[0]);

        // read string from pipe
        int str_len = read(fd1[0], buffer2, buffer_size);
        close(fd1[0]);

        // find the substring
        char answer_pos[5];
        int digits = find_substr(buffer2, argv[3], answer_pos);

        // send answer to next process
        int bytes = write(fd2[1], &answer_pos, digits);
        close(fd2[1]);

        if (bytes != digits) {
            printf("The answer was transfered with errors\n");
            exit(-1);
        }
    }

    return 0;
}