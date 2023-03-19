#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

const int buffer_size = 5001;

// First process that reads initial data from input and prints answer to output

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("2 arguments needed: input file, output file\n");
        exit(-1);
    }

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
}
