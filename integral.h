
#include "basic.h"

// 1-variable definite integral from a to b divided into [intervals] parts.
// computes across [parallels] processes
// recommended but not required that intervals be a multiple of parallels
double evalDefiniteIntegral(Expression exp, char variable, double a, double b,
    int intervals, int parallels);

#define EXIT_FAILURE 1

// use for major status changes and minor errors
#define STATUS(format,...) if(!quiet)fprintf(stderr,format "\n",__VA_ARGS__)
// use for minor progress reports
#define PROGRESS(format,...) if(verbose)fprintf(stderr,format "\n",__VA_ARGS__)

// no newline, string literal
#define PROGRESS_PART(format) if(verbose)fprintf(stderr,format)

// Write message to stderr using format FORMAT
#define WARN(format,...) fprintf (stderr, format "\n", __VA_ARGS__)

// Write message to stderr using format FORMAT and exit.
#define DIE(format,...)  WARN(format,__VA_ARGS__), exit (EXIT_FAILURE)

// call after system call fails to print error associated with errno
#define SYS_ERROR(name) if(!quiet)perror(name)
// system error followed by return from current function
#define SYS_ERR_DONE(name) {SYS_ERROR(name);return;}
// fatal system error
#define SYS_DIE(name) perror(name),exit(EXIT_FAILURE)

#define STAT(x) (WIFEXITED(x) ? WEXITSTATUS(x) : 128+WTERMSIG(x))