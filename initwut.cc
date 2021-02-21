#if 0
gcc=g++
src=initwut.cc
out=initwut
echo "Compiling $out..."
$gcc -o $out $src
echo "Note: requires in, test."
exit 0
#else

#include <iostream>
#include <stdio.h>
using namespace std;

//#include "initshift.h" // not with popen mode/yet

//#undef INPUTPATH // .h defines for in/ subdir
//#define INPUTPATH  "./in"
#define INPUTPATH  "./test 9"
#define TESTPATH   "./test"
#define TESTKBD1   "9"
#define TESTKBD2   "12"

// THIS IS NOT NONBLOCKING ...... BUT WILL UPDATE SHIFT VARS ???? FORK THEN POPEN ????

int main(int argc, char **argv) {
  cout << "HELLO\n";
  FILE *input = popen(INPUTPATH, "r");
  cout << "HELLO\n";
  if (!input) { return 1; }
  cout << "HELLO\n";
  while (1) {
  cout << "HELLO\n";
    fflush(input);
    int inch = fgetc(input);
    if (inch == EOF) { break; }
    cout << "(" << (char)inch << ")\n";
  }
  pclose(input);
  return 0;
}




/*


#include <errno.h>
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    pid_t pid1, pid2;
    int a[2];
    int b[2];
    pipe(a);

    pid1 = fork();

    if (pid1 < 0) {
        fprintf(stderr, "failed to fork child 1 (%d: %s)\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
        close(a[1]);    // Must be closed before the loop
        FILE *stream = fdopen(a[0], "r");
        if (stream == NULL) {
            fprintf(stderr, "failed to create stream for reading (%d: %s)\n", errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
// need to run execl for an amount of time .... in should do it ?
        // echo back any keys given on input
        int c;
        while ((c = getc(stream)) != EOF)
            putchar(c);

        // then send in keys
//        execl(INPUTPATH, INPUTPATH, NULL);
        execl(TESTPATH, TESTPATH, TESTKBD1, NULL);


        //char x;
        //fscanf(stream, "%c", &x);
        //printf("%c\n", x);

        //close(a[0]);  -- Bad idea once you've used fdopen() on the descriptor



        printf("Child 1 done\n");
        exit(0);
    } else {
        pipe(b);
        pid2 = fork();
        if (pid2 < 0) {
            fprintf(stderr, "failed to fork child 2 (%d: %s)\n", errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
        else if (pid2 == 0) {
            close(b[1]);
            close(a[1]);
            close(a[0]);
            FILE *stream = fdopen(b[0], "r");
            if (stream == NULL) {
                fprintf(stderr, "failed to create stream for reading (%d: %s)\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
            }

            int c;
            while ((c = getc(stream)) != EOF)
                putchar(c);

            execl(TESTPATH, TESTPATH, TESTKBD2, NULL);
            //char x;
            //fscanf(stream, "%c", &x);
            //printf("%c\n", x);

            printf("Child 2 done\n");
            exit(0);
        }
    }

    close(a[0]);
    close(b[0]);

    FILE *stream1 = fdopen(a[1], "w");
    if (stream1 == NULL) {
        fprintf(stderr, "failed to create stream for writing (%d: %s)\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    FILE *stream2 = fdopen(b[1], "w");
    if (stream2 == NULL) {
        fprintf(stderr, "failed to create stream for writing (%d: %s)\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    fprintf(stream1, "yo bella zio\n");
    fprintf(stream2, "como estas\n");

    fflush(stream1);    // Not necessary because fclose flushes the stream
    fflush(stream2);    // Not necessary because fclose flushes the stream
    fclose(stream1);    // Necessary because child won't get EOF until this is closed
    fclose(stream2);    // Necessary because child won't get EOF until this is closed
    //close(a[1]);      -- bad idea once you've used fdopen() on the descriptor
    //close(b[1]);      -- bad idea once you've used fdopen() on the descriptor

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    printf("All done!\n");
    return 0;
}



*/




#endif

