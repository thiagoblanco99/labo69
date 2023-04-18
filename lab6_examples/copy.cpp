#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
using namespace std;

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3 || strcmp(argv[1], "--help") == 0) {
        cout << "Usage: " << argv[0] << " old-file new-file\n";
        return 0;
    }
    /* Open input and output files */
    int inputFd = open(argv[1], O_RDONLY);
    if (inputFd == -1) {
        cout << "Error: Opening file " << argv[1] << endl;
        return -1;
    }
    int openFlags = O_CREAT | O_WRONLY | O_TRUNC;
    mode_t filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH; /* rw-rw-rw- */
    int outputFd = open(argv[2], openFlags, filePerms);
    if (outputFd == -1) {
        cout << "Error: Opening file " << argv[2] << endl;
        return -1;
    }
    /* Transfer data until we encounter end of input or an error */
    size_t numRead;
    char buf[BUF_SIZE];
    while ((numRead = read(inputFd, buf, BUF_SIZE)) > 0)
        if (write(outputFd, buf, numRead) != numRead) {
            cout << "Error: couldn't write whole buffer";
            exit(1);
        }
    if (numRead == -1) {
        cout << "Error: reading\n";
        exit(1);
    }
    if (close(inputFd) == -1) {
        cout << "Error: closing input\n";
        exit(1);
    }
    if (close(outputFd) == -1) {
        cout << "Error: closing output\n";
        exit(1);
    }
    return 0;
}
