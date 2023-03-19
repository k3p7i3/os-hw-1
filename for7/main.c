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

    if (argc < 4) {
        printf("3 arguments needed: input file, output file, substring\n");
        exit(-1);
    }

    pid_t chpid1 = fork();
    if (chpid1 == -1) {
        printf("Can't create child process\n");
        exit(-1);
    }
    if (chpid1 > 0) {
        // the "first" process that reads string from input file and prints answer to output file
        char buffer1[buffer_size];

        // read string from input file
        int input_file = open(argv[1], O_RDONLY);

        if (input_file == -1) {
            printf("Can't open input file %s\n", argv[1]);
            exit(-1);
        }
        int str_len = read(input_file, buffer1, buffer_size);
        close(input_file);

        // create named channel FIFO1 if not exist
        umask(0);
        mknod("FIFO1", S_IFIFO | 0666, 0);

        // write initial data to FIFO1 to send it to 2nd process
        int fp = open("FIFO1", O_WRONLY | O_CREAT | O_TRUNC);
        if (fp == -1) {
            printf("Can't open FIFO1 channel for writing\n");
            exit(-1);
        }

        int written_len = write(fp, buffer1, str_len);
        if (str_len != written_len) {
            printf("Some parts of string were lost while writing!\n");
            exit(-1);
        }
        close(fp);

        // next part - get answer from 2nd process and write it to output
        // create named channel FIFO2 if not exist
        umask(0);
        mknod("FIFO2", S_IFIFO | 0666, 0);

        // read answer from FIFO2
        fp = open("FIFO2", O_RDONLY);
        if (fp == -1) {
            printf("Can't open FIFO2 channel for reading\n");
            exit(-1);
        } 
        char answer_pos[5];
        int digits = read(fp, &answer_pos, 5);
        close(fp);

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

        // create named channel FIFO1 if not exist
        umask(0);
        mknod("FIFO1", S_IFIFO | 0666, 0);

        // read initial string from pipe FIFO1
        int fp = open("FIFO1", O_RDONLY);
        if (fp == -1) {
            printf("Can't open FIFO1 channel for reading\n");
            exit(-1);
        }

        int str_len = read(fp, buffer2, buffer_size);
        close(fp);

        // find the substring
        char answer_pos[5];
        int digits = find_substr(buffer2, argv[3], answer_pos);

        // create 2nd named channel FIFO2 if not exist
        umask(0);
        mknod("FIFO2", S_IFIFO | 0666, 0);

        // and send answer to first process back
        fp = open("FIFO2", O_WRONLY | O_CREAT | O_TRUNC);
        if (fp == -1) {
            printf("Can't open FIFO2 channel for writing\n");
            exit(-1);
        }
        int bytes = write(fp, &answer_pos, digits);

        if (bytes != digits) {
            printf("The answer was transfered with errors\n");
            exit(-1);
        }
        close(fp);

    }

    return 0;
}