# Job Sorter

This program takes in a formatted list of "jobs" and their arrival times and implements a shortest-job-first algorithm
to sort them.

# Usage

```
$ gcc -Wall -o Job-Sorter Job-Sorter.c
$ ./Job-Sorter < Sample-Input.txt
```

This program is not that useful in real world scenarios, it is just a struct-sorting algorithm wrapped in the guise of job handling. But, on the off chance that a list of jobs is available and you need to know when
a given user's last job was finished on the CPU, this could be useful.

# Sample Input

See Sample-Input.txt for specific style, but the format is the following.

| USER NAME | PROCESS/JOB NAME | ARRIVAL TIME | DURATION |
| --------- | ---------------- | ------------ | -------- |
| Jim       | A                | 2            | 5        |
| Mary      | B                | 2            | 3        |
| Sue       | D                | 5            | 5        |
| Mary      | C                | 6            | 2        |

The duration is difficult to know in real world scenarios, but this program takes it for granted. It could be that the duration was
calculated by averaging previous executions, or it was just guessed based on some user-defined parameters. This program does not
care how the value was found, just that there is a value available.

# Sample Output

See out.txt for the exact style.

| TIME | JOB  |
| ---- | ---- |
| 2    | B    |
| 3    | B    |
| 4    | B    |
...
| 15  | D    |
| 16  | D    |
| 17  | IDLE |

| Summary |     |
| ------- | --- |
| Jim     | 12  |
| Mary    | 8   |
| Sue     | 17  |

The output will show which time slots get which jobs, up until all the jobs have finished. It then gives a summary of when each person's last job is finished. Mary had two jobs, so it shows when job C finished, which was the last job they ran.
