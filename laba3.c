#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>

#define MAX_CHILDREN 100

typedef struct {
    int val1;
    int val2;
} Data;

typedef struct {
    pid_t pid;
    int active;
} ChildInfo;

ChildInfo children[MAX_CHILDREN];
int child_count = 0;
volatile sig_atomic_t can_print = 1;

/* --- ЛОГИКА ДОЧЕРНЕГО ПРОЦЕССА --- */

void child_routine(int id) {
    Data data = {0, 0};
    long stats[2][2] = {{0, 0}, {0, 0}};
    struct timespec ts = {0, 10000000}; // 10 мс
    int iterations = 0;

    while (1) {
        /* "Вечный" цикл изменения данных */
        for (int j = 0; j < 10000; j++) {
            data.val1 = 0;
            data.val2 = 0;
            data.val1 = 1;
            data.val2 = 1;
        }

        /* Проверка состояния после "будильника" (имитация через nanosleep) */
        nanosleep(&ts, NULL);
        stats[data.val1][data.val2]++;
        iterations++;

        /* Вывод статистики каждые 101 итерацию */
        if (iterations >= 101) {
            printf("CHILD %02d: PPID=%-5d PID=%-5d | 00:%-3ld 01:%-3ld 10:%-3ld 11:%-3ld\n",
                   id, getppid(), getpid(),
                   stats[0][0], stats[0][1], stats[1][0], stats[1][1]);

            /* Сброс статистики */
            memset(stats, 0, sizeof(stats));
            iterations = 0;
        }
    }
}

/* --- ЛОГИКА РОДИТЕЛЬСКОГО ПРОЦЕССА --- */

void kill_all() {
    for (int i = 0; i < child_count; i++) {
        if (children[i].active) {
            kill(children[i].pid, SIGTERM);
            waitpid(children[i].pid, NULL, 0);   // забираем статус завершения
            children[i].active = 0;
        }
    }
    printf("PARENT: All children killed.\n");
}

int main() {
    printf("Commands: + (add), - (remove last), l (list), k (kill all), q (quit)\n");

    char cmd;
    while (scanf(" %c", &cmd) == 1) {
        if (cmd == '+') {
            if (child_count < MAX_CHILDREN) {
                pid_t pid = fork();

                if (pid < 0) {
                    perror("fork");
                } 
                else if (pid == 0) {
                    child_routine(child_count);
                    exit(0);
                } 
                else {
                    children[child_count].pid = pid;
                    children[child_count].active = 1;
                    child_count++;
                    printf("PARENT: Added child %d (Total: %d)\n", pid, child_count);
                }
            }
        } 
        else if (cmd == '-') {
            if (child_count > 0) {
                child_count--;
                kill(children[child_count].pid, SIGTERM);
                waitpid(children[child_count].pid, NULL, 0);   // чтобы не было зомби
                children[child_count].active = 0;
                printf("PARENT: Removed last child. Remaining: %d\n", child_count);
            }
        } 
        else if (cmd == 'l') {
            printf("PARENT PID: %d\n", getpid());
            for (int i = 0; i < child_count; i++) {
                if (children[i].active) {
                    printf("  Child [%d]: PID %d\n", i, children[i].pid);
                }
            }
        } 
        else if (cmd == 'k') {
            kill_all();
            child_count = 0;
        } 
        else if (cmd == 'q') {
            kill_all();
            exit(0);
        }
    }

    return 0;
}
