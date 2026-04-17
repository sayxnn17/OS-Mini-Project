// This program demonstrates fault tolerance by simulating a crash mid-execution
// and showing how the system resumes from the last saved progress when restarted.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PROGRESS_FILE "/tmp/progress.txt"

int main() {
    int start_chunk = 1;
    long long total_data_processed = 0;

    FILE *f = fopen(PROGRESS_FILE, "r");

    if (f != NULL) {
        fscanf(f, "%d %lld", &start_chunk, &total_data_processed);
        fclose(f);
        printf("[RESUMING] From chunk %d\n", start_chunk);
    } else {
        printf("[STARTING NEW TASK]\n");
    }

    for(int chunk = start_chunk; chunk <= 10; chunk++) {

        sleep(1);
        total_data_processed += 1000;

        printf("Chunk %d done\n", chunk);

        f = fopen(PROGRESS_FILE, "w");
        if (f) {
            fprintf(f, "%d %lld", chunk + 1, total_data_processed);
            fclose(f);
        }

        if (chunk == 5 && start_chunk == 1) {
            printf("CRASHING...\n");
            exit(1);
        }
    }

    printf("DONE: %lld\n", total_data_processed);
    remove(PROGRESS_FILE);
    return 0;
}
