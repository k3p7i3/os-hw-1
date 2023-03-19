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

// Second process that processes data

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Substring wasn't entered\n");
        exit(-1);
    }

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
    int digits = find_substr(buffer2, argv[1], answer_pos);

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

    return 0;
}