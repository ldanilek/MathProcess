#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include "integral.h"
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>

bool rdhang(int fd, char* bytes, int len)
{
    if (len == 0) return true;
    int lengthRead = 0;
    int totalRead = 0;
    while (len > 0 && (lengthRead = read(fd, bytes + totalRead, len)) > 0)
    {
        len -= lengthRead;
        totalRead += lengthRead;
    }
    if (lengthRead < 0) SYS_DIE("read"); // read had an error
    if (lengthRead == 0 && len > 0 && totalRead > 0)
    {
        // hit EOF; read in some but not all
        DIE("%d spare bytes", totalRead);
    }
    return len == 0; // return whether everything was read
}

// executes in one process
double evalDefiniteIntegralHere(Expression exp, char variable, double a,
    double b, int intervals)
{
    double result = 0;
    double variables[128];
    double intervalWidth = (b - a) / intervals;
    for (int i = 0; i < intervals; i++)
    {
        double start = a + ((double)i / (double)intervals) * (b - a);
        double end = a + ((double)(i+1) / (double)intervals) * (b - a);
        variables[variable] = (start+end) / 2;
        result += evalExpression(exp, variables) * intervalWidth;
    }
    return result;
}

double evalDefiniteIntegral(Expression exp, char variable, double a, double b,
    int intervals, int parallels)
{
    // writes of size <= PIPE_BUF are atomic
    // PIPE_BUF is 512, so I can write doubles to a pipe from multiple processes
    int outputPipe[2];
    if (pipe(outputPipe)) perror("fork"), exit(errno);
    // printf("PIPE_BUF is %d\n", (int)PIPE_BUF);
    int intervalsPerParallel = intervals/parallels;
    pid_t pids[parallels];
    for (int i = 0; i < parallels; i++)
    {
        pids[i] = fork();
        if (pids[i] < 0) perror("fork"), exit(errno);
        if (pids[i] == 0)
        {
            close(outputPipe[0]);
            double start = a + ((double)i / (double)parallels) * (b - a);
            double end = a + ((double)(i+1) / (double)parallels) * (b - a);
            if (i == parallels-1)
            {
                intervalsPerParallel = intervals - intervalsPerParallel * i;
            }
            double result = evalDefiniteIntegralHere(exp, variable, start, end,
                intervalsPerParallel);
            write(outputPipe[1], &result, sizeof(result));
            exit(0);
        }
    }
    close(outputPipe[1]);
    double result = 0;
    double readDouble = 0;
    while (rdhang(outputPipe[0], (char*)(&readDouble), sizeof(readDouble)))
    {
        result += readDouble;
    }

    // reap all zombies
    for (int i = 0; i < parallels; i++)
    {
        int status;
        waitpid(pids[i], &status, 0);
        if (status) DIE("Subprocess had status %d", STAT(status));
    }
    return result;
}