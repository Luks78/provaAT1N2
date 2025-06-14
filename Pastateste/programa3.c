#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_SENSORS 10
#define READINGS_PER_SENSOR 2000

typedef struct {
    char id[50];
    char type[20];
} Sensor;

time_t parse_datetime(const char *datetime_str) {
    struct tm tm = {0};
    sscanf(datetime_str, "%d-%d-%d %d:%d:%d",
           &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
           &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    return mktime(&tm);
}

double generate_random_value(const char *type) {
    if (strcmp(type, "int") == 0) {
        return rand() % 100;
    } else if (strcmp(type, "float") == 0) {
        return (double)rand() / RAND_MAX * 100.0;
    } else if (strcmp(type, "boolean") == 0) {
        return rand() % 2;
    } else {
        return (double)rand() / RAND_MAX * 100.0;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 6 || (argc - 3) % 2 != 0) {
        fprintf(stderr, "Uso: %s <inicio_YYYY-MM-DD HH:MM:SS> <fim_YYYY-MM-DD HH:MM:SS> <sensor1> <tipo1> [<sensor2> <tipo2> ...]\n", argv[0]);
        return 1;
    }

    srand((unsigned int)time(NULL));

    time_t start_time = parse_datetime(argv[1]);
    time_t end_time = parse_datetime(argv[2]);
    double time_range = difftime(end_time, start_time);

    int num_sensors = (argc - 3) / 2;
    if (num_sensors > MAX_SENSORS) {
        fprintf(stderr, "Erro: número máximo de sensores suportados é %d.\n", MAX_SENSORS);
        return 1;
    }
    Sensor sensors[MAX_SENSORS];

    for (int i = 0; i < num_sensors; i++) {
        strncpy(sensors[i].id, argv[3 + i*2], sizeof(sensors[i].id) - 1);
        sensors[i].id[sizeof(sensors[i].id) - 1] = '\0';
        strncpy(sensors[i].type, argv[4 + i*2], sizeof(sensors[i].type) - 1);
        sensors[i].type[sizeof(sensors[i].type) - 1] = '\0';
    }

    FILE *output_file = fopen("sensor_data.txt", "w");
    if (!output_file) {
        perror("Erro ao criar arquivo de saída");
        return 1;
    }

    for (int i = 0; i < num_sensors; i++) {
        for (int j = 0; j < READINGS_PER_SENSOR; j++) {
            time_t random_time = start_time + (time_t)((double)rand() / RAND_MAX * time_range);
            double value = generate_random_value(sensors[i].type);

            if (strcmp(sensors[i].type, "boolean") == 0) {
                fprintf(output_file, "%ld %s %s\n", random_time, sensors[i].id, ((int)value) ? "true" : "false");
            } else if (strcmp(sensors[i].type, "int") == 0) {
                fprintf(output_file, "%ld %s %d\n", random_time, sensors[i].id, (int)value);
            } else {
                fprintf(output_file, "%ld %s %.2f\n", random_time, sensors[i].id, value);
            }
        }
    }

    fclose(output_file);
    printf("Arquivo 'sensor_data.txt' gerado com exito\n");

    return 0;
}