// Shir Fintsy

#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>

int checkIfAbsPath(char path[]) {
    // check if exists:
    if ((open(path, O_RDONLY)) < 0)
        return 0; // not absolute path
    //printf("can open\n");
    return 1; // absolute path
}

void writeToResults(char string[]) {
    int resultsFile = open("results.csv", O_WRONLY | O_APPEND | O_CREAT, 0777);
    if (resultsFile < 0){
        char error[] = "Error in open\n";
        write(1, error, strlen(error));
    }
    unsigned long len = strlen(string);
    if ((write(resultsFile, string, len)) < 0)
        return;
    if((close(resultsFile)) < 0){
        char error[] = "Error in close\n";
        write(1, error, strlen(error));
    }
}

void getAbsPath(char relPath[], char path[], char destPath[]) {
    if (checkIfAbsPath(relPath) == 1){
        strcpy(destPath, relPath);
        return;
    }
    //printf(" rel path: %s   path: %s\n", relPath, path);
    strcat(destPath, path);
    strcat(destPath, "/");
    strcat(destPath, relPath);
}

int isCFile(char file[]) {
    unsigned long len = strlen(file);
    if (file[len - 1] == 'c' && file[len - 2] == '.')
        return 1;
    return 0;
}

void compileFile(char path[]) {
    // compile the file:
    char * tab[] = {"gcc", path, NULL };
    if (execvp("gcc",tab) < 0) {
        char error[] = "Error in execvp\n";
        write(1, error, strlen(error));
    }

}

int compileComp(char outputFile[]) {
    int stat, result = 0;
    char* tabComp[] = {"./comp.out", outputFile, "output.txt", NULL};
    pid_t pid = fork();

    if (pid == -1) { // the fork failed
        char error[] = "Error in fork\n";
        write(1, error, strlen(error));
    } else if (pid == 0) { // the child
        if (execvp(tabComp[0],tabComp) < 0) {
            char error[] = "Error in execvp\n";
            write(1, error, strlen(error));
        }
    } else {
    // waiting for child to terminate
    waitpid(pid, &stat, 0);
    result = WEXITSTATUS(stat);
    }
    return result;
}

// use ex21 to compare between the output file and the expected output file given.
void checkSimilarity(char name[],char outputFile[]) {
    int returnValue = compileComp(outputFile);
    if (returnValue == 1)
        writeToResults(strcat(name,"100,EXCELLENT\n"));
    else if (returnValue == 2)
        writeToResults(strcat(name,"50,WRONG\n"));
    else if (returnValue ==3)
        writeToResults(strcat(name,"75,SIMILAR\n"));
}



void compareFiles(char name[], char inputFile[], char outputFile[]) {
    int in, out;
    char* tab[] = {"./a.out", NULL};
    pid_t pid = fork();

    if (pid == -1) { // the fork failed
        char error[] = "Error in fork\n";
        write(1, error, strlen(error));
        exit(-1);
    } else if (pid == 0) { // the child
        in = open(inputFile, O_RDONLY, 0666);
        if (in < 0){
            char error[] = "Error in open\n";
            write(1, error, strlen(error));
        }
        out = open("output.txt", O_WRONLY | O_TRUNC | O_CREAT, 0777);
        if (out < 0) {
            char error[] = "Error in open\n";
            write(1, error, strlen(error));
        }
        dup2(in, 0); // the inputs for a file will be the content of the input file given
        dup2(out, 1); // the output for a file will be writen in output.txt file
        if (close(in) < 0) {
            char error[] = "Error in close\n";
            write(1, error, strlen(error));
        }
        if (close(out) < 0) {
            char error[] = "Error in close\n";
            write(1, error, strlen(error));
        }

        if (execvp("./a.out", tab) < 0) {
            char error[] = "Error in execvp\n";
            write(1, error, strlen(error));
        }
    } else {
        // waiting for child to terminate
        wait(NULL);
        checkSimilarity(name, outputFile);
        return;
    }
}

int handelFile(char path[]) {
    int fdError = open("errors.txt", O_WRONLY | O_CREAT, 0644);
    if (fdError < 0){
        char error[] = "Error in open\n";
        write(1, error, strlen(error));
    }
    dup2(fdError, 2); // all errors will be writen in error.txt file.
    if (close(fdError) < 0) {
        char error[] = "Error in close\n";
        write(1, error, strlen(error));
    }

    int stat;
    // Forking a child- because it is a c file
    pid_t pid = fork();

    if (pid == -1) { // the fork failed
        char error[] = "Error in fork\n";
        write(1, error, strlen(error));
        exit(-1);
    } else if (pid == 0) { // the child
        compileFile(path);
    } else {
        // waiting for child to terminate
        wait(&stat);
        if (stat != 0) { // file didn't compiled
            return 0;
       }
    }
    return 1;
}


int isDir(char path[]) {
    struct stat path_stat;
    stat(path, &path_stat);
    if (S_ISREG(path_stat.st_mode) == 1) {
        return 0;
    }
    return 1;
}
/*
 * for every subdirectory we found we're trying to compile and compare it with the expected output
 */
void handelSubdir(char path[], char name[], char input[], char output[]) {
    DIR* dip;
    struct dirent* dit;
    int flagForCFile = 0;

    // open dir (and check if exists):
    if ((dip = opendir(path)) == NULL) {
        char error[] = "Error in opendir\n";
        write(1, error, strlen(error));
    }

    while((dit = readdir(dip)) != NULL) { // get all files from current directory
        // get absolute path
        char absSubPath[150] = "";
        getAbsPath(dit->d_name, path, absSubPath);
        printf("current file: %s\n", dit->d_name);
        // check that it's not a file or the directory is . or .. and make sure it's a C file
        if (strcmp(dit->d_name, ".") != 0 && strcmp(dit->d_name, "..") != 0 && isDir(absSubPath) == 0
        && isCFile(absSubPath) == 1) {
            flagForCFile = 1;
            int handle = handelFile(absSubPath);
            if (handle == 1) {
                compareFiles(name, input, output);
            } else {
                writeToResults(name);
                writeToResults("10,COMPILATION_ERROR\n");
            }
        }
    }
    if (flagForCFile == 0)
        writeToResults(strcat(name, "0,NO_C_FILE\n"));

    if(closedir(dip) < 0) {
        char error[] = "Error in closedir\n";
        write(1, error, strlen(error));
    }

}

/*
 * flag: 1-> 1st line, 2-> 2nd line(input), 3-> 3rd line (output);
 */
void checkValidation(char path[], int flag) {
    // make sure that path is directory:
    if (isDir(path) == 0 && flag == 1){
        char error[] = "Not a valid directory\n";
        write(1, error, strlen(error));
        exit(-1);
    } else if (isDir(path) == 1 && flag == 2){
        char error[] = "Input file not exist\n";
        write(1, error, strlen(error));
        exit(-1);
    } else if (isDir(path) == 1 && flag == 3){
        char error[] = "Output file not exist\n";
        write(1, error, strlen(error));
        exit(-1);
    }
}

void readingFromFD(char from[], char path[], char input[], char output[]) {
    int fd = open(from, O_RDONLY);
    if (fd < 0) {
        char error[] = "Error in open\n";
        write(1, error, strlen(error));
    }
    int count = 0;
    // get the path given from configuration file - first line.
    while (read(fd , &path[count], sizeof(char)) > 0) {
        if (path[count] == '\n') {
            path[count] = '\0';
            break;
        }
        count++;
    }
    checkValidation(path, 1);
    count = 0;
    // get the path given from configuration file - second line.
    while (read(fd , &input[count], sizeof(char)) > 0) {
        if (input[count] == '\n') {
            input[count] = '\0';
            break;
        }
        count++;
    }
    checkValidation(input, 2);
    count = 0;
    // get the path given from configuration file - third line.
    while (read(fd , &output[count], sizeof(char)) > 0) {
        if (output[count] == '\n') {
            output[count] = '\0';
            break;
        }
        count++;
    }
    checkValidation(output, 3);

    if (close(fd) < 0) {
        char error[] = "Error in close\n";
        write(1, error, strlen(error));
    }
}

void checkIdUsersIsOnlyDirs(char path[]) {
    DIR *dip;
    struct dirent *dit;
    if ((dip = opendir(path)) == NULL) {
        char error[] = "Error in opendir\n";
        write(1, error, strlen(error));
        exit(-1);
    }

    int i = 0;
    // run on all subdirectory in path in line 1 of configuration file
    while((dit = readdir(dip)) != NULL) {
        char absPath[150] = "";
        getAbsPath(dit->d_name, path, absPath);
        // check that the directory is not . or .. or file:
        if (strcmp(dit->d_name, ".") != 0 && strcmp(dit->d_name, "..") != 0 && isDir(absPath) == 0) {
            char error[] = "Not a valid directory\n";
            write(1, error, strlen(error));
            exit(-1);
        }
    }
    // closing the users directory we got from config file
    if (closedir(dip) < 0){
        char error[] = "Error in closedir\n";
        write(1, error, strlen(error));
        exit(-1);
    }
}

int main(int argc, char *argv[]) {
    DIR *dip;
    struct dirent *dit;
    char inputCpy[150], outputCpy[150], pathCpy[150];
    char inputFile[150], outputFile[150], path[150], cwd[PATH_MAX];
    if (argc < 2)
        exit(-1);
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        char error[] = "Error in getcwd\n";
        write(1, error, strlen(error));
    }

    readingFromFD(argv[1], pathCpy, inputCpy, outputCpy); // set the path of the files given.
    // init absolute path if needed:
    getAbsPath(pathCpy, cwd, path);
    getAbsPath(inputCpy, cwd, inputFile);
    getAbsPath(outputCpy, cwd, outputFile);

    // check if directory is exists and open it:
    if ((dip = opendir(path)) == NULL) {
        char error[] = "Error in opendir\n";
        write(1, error, strlen(error));
        exit(-1);
    }

    //check if user directory contain only directories:
    checkIdUsersIsOnlyDirs(path);


    int i = 0;
    // run on all subdirectory in path in line 1 of configuration file
    while((dit = readdir(dip)) != NULL) {
        char absPath[150] = "";
        getAbsPath(dit->d_name, path, absPath);
        // check that the directory is not . or .. or file:
        if (strcmp(dit->d_name, ".") != 0 && strcmp(dit->d_name, "..") != 0 && isDir(absPath) == 1) {
            printf("NAME: %s\n", dit->d_name); //debug
            char* name = strcat(dit->d_name, ",");
            //writeToResults(name);
            i++;
            handelSubdir(absPath, name, inputFile, outputFile);
        }
    }
    // closing the directory we got from config file
    if (closedir(dip) < 0){
        char error[] = "Error in closedir\n";
        write(1, error, strlen(error));
        exit(-1);
    }
    if ((open("output.txt", O_RDONLY)) > 0) { // check if exists.
        if (remove("output.txt") != 0) { // because it exists we need to delete the file.
            char error[] = "Error in remove\n";
            write(1, error, strlen(error));
            exit(-1);
        }
    }
}
