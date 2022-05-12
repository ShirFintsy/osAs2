// Shir Fintsy 206949075

#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

/*
 * Check if the given args are files.
 */
void checkIfFile(char *argv[]) {
    struct stat fileStat;
    for (int i = 0; i < 2; ++i) {
        if (stat(argv[i], &fileStat) < 0)
            exit(0);
        // if one of the arguments to main is not file
        if (S_ISREG(fileStat.st_mode) == 0)
            exit(0);
    }

}

/*
 * Return 1 if one of the characters is the capital letter to the second one.
 * Return 0 otherwise.
 */
int checkCapital(char c1, char c2) {
    //printf("in capital func--> char1: \"%s\", char2: \"%s\"\n",c1, c2);
    if (c1 - c2 == 32 || c2 -c1 == 32){
        return 1;
    }
    return 0;
}

/*
 * Return 1 if character 1 is empty char, 2 if character 2 is empty char
 * and 0 otherwise.
 */
int emptyChars(char c1, char c2) {
    //printf("in empty func-> char1: \"%s\", char2: \"%s\"\n",c1, c2);
    if (c1 == '\n' || c1 == ' ')
        return 1;
    if (c2 == '\n' || c2 == ' ')
        return 2;
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3)
        exit(0);
    checkIfFile(argv); // make sure we got files paths
    int fd1, fd2;

    fd1 = open(argv[1], O_RDONLY);
    if (fd1 < 0)
        return -1;
    fd2 = open(argv[2], O_RDONLY);
    if (fd2 < 0)
        return -1;
    // the flags are for notify not to read another char because the last char was empty char.
    // ri are for the read
    // similarity is for notify that the files may be similar (but defiantly not identical)
    int flag1 = 0, flag2 = 0, r1, r2, similarity = 0, count1 = 0, count2;
    char buff1[100], buff2[100];
    while(1) {
        if (flag1 == 0) {
            r1 = read(fd1 , &buff1[count1], sizeof(char));
            if (r1 < 0)
                return -1;
        }
        flag1 = 0;

        if (flag2 == 0) {
            r2 = read(fd2 , &buff2[count2], sizeof(char));
            if (r2 < 0)
                return -1;
        }
        flag2 = 0;
        //printf("all char-->  char1: \"%c\", char2: \"%c\"\n", buff1[count1], buff2[count2]);
        //printf("   all reads: fisrt: %d, second: %d\n", r1, r2);
        // if both files finished reading at the same time - they are identical

        if (r1 == 0 && r2 == 0){
            if (similarity == 1) {
                //printf("return 3!\n");
                return 3;
            }
            //printf("return 1!\n");
            return 1;
        }


        // if only one of the file ended then they are not equal (not the same length)
        if (r1 == 0 || r2 == 0){
            if (emptyChars(buff1[count1], buff2[count2]) == 1 || emptyChars(buff1[count1], buff2[count2]) == 2) {
                //printf("return 3!\n");
                return 3;
            }
            //printf("return 2!\n");
            return 2;
        }

        // the chars are not equals
        if (buff1[count1] != buff2[count2]) {
            //printf("in comp--> char1: \"%c\", char2: \"%c\"\n",buff1[count1], buff2[count2]);
            if (emptyChars(buff1[count1], buff2[count2]) == 1) { // c1 is empty char
                //printf("first empty--> char1: \"%c\", char2: \"%c\"\n",buff1[count1], buff2[count2]);
                flag2 = 1;
                similarity = 1;
                count2--;

            } else if (emptyChars(buff1[count1], buff2[count2]) == 2) {// c2 is empty char
                //printf("second empty--> char1: \"%c\", char2: \"%c\"\n",buff1[count1], buff2[count2]);
                flag1 = 1;
                similarity = 1;
                count1--;

            } else if (checkCapital(buff1[count1], buff2[count2]) == 1){ // emptyChar return 0 and it's the capital condition
                similarity = 1;
            } else{return 2;} // emptyChar return 0 and it's not the capital condition

        }
        count1++, count2++;
    }
}
