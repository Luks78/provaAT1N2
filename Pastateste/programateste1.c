#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#define mkdir _mkdir
#endif
#define MAX_LINE_LENGTH 256
#define MAX_SENSORS 100
#define MAX_SENSOR_ID_LENGTH 50

typedef struct {
    long timestamp;
    char value_str[MAX_LINE_LENGTH];
    int value_int;
    float value_float;
    int value_bool;
} SensorReading;

typedef struct {
    char id[MAX_SENSOR_ID_LENGTH];
    int type; 
    SensorReading *readings;
    int count;
    int capacity;
} SensorData;

int determine_value_type(const char *value) {
    char *endptr;
    
    
    strtol(value, &endptr, 10);
    if (*endptr == '\0') return 0;
    
    
    strtof(value, &endptr);
    if (*endptr == '\0') return 1;
    
    
    if (strcmp(value, "true") == 0 || strcmp(value, "false") == 0) return 2;
    
     
    int len = strlen(value);
    if (len <= 16) {
        int is_alpha = 1;
        for (int i = 0; i < len; i++) {
            if (!isalpha(value[i])) {
                is_alpha = 0;
                break;
            }
        }
        if (is_alpha) return 3;
    }
    
    return -1; 
}

void add_reading(SensorData *sensor, long timestamp, const char *value_str) {
    if (sensor->count >= sensor->capacity) {
        sensor->capacity = (sensor->capacity == 0) ? 1 : sensor->capacity * 2;
        SensorReading *tmp = realloc(sensor->readings, sensor->capacity * sizeof(SensorReading));
        if (!tmp) {
            fprintf(stderr, "Memory allocation failed for sensor %s\n", sensor->id);
            free(sensor->readings);
            exit(EXIT_FAILURE);
        }
        sensor->readings = tmp;
    }
    
    SensorReading *r = &sensor->readings[sensor->count++];
    r->timestamp = timestamp;
    strncpy(r->value_str, value_str, MAX_LINE_LENGTH-1);
    r->value_str[MAX_LINE_LENGTH-1] = '\0';
    
    switch (sensor->type) {
        case 0: r->value_int = atoi(value_str); break;
        case 1: r->value_float = atof(value_str); break;
        case 2: r->value_bool = (strcmp(value_str, "true") == 0); break;
        case 3: break; 
    }
}

int compare_readings(const void *a, const void *b) {
    const SensorReading *ra = (const SensorReading *)a;
    const SensorReading *rb = (const SensorReading *)b;
    return (ra->timestamp > rb->timestamp) - (ra->timestamp < rb->timestamp);
}

void process_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    SensorData sensors[MAX_SENSORS] = {0};
    int sensor_count = 0;
    
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        
        if (line[0] == '\n' || line[0] == '\0') continue;
        
       
        long timestamp;
        char sensor_id[MAX_SENSOR_ID_LENGTH];
        char value[MAX_LINE_LENGTH];
        
        if (sscanf(line, "%ld %49s %255[^\n]", &timestamp, sensor_id, value) != 3) {
            fprintf(stderr, "Warning: Malformed line: %s", line);
            continue;
        }
        
        
        int value_type = determine_value_type(value);
        if (value_type == -1) {
            fprintf(stderr, "Warning: Unknown value type for sensor %s: %s\n", sensor_id, value);
            continue;
        }
        
        
        SensorData *sensor = NULL;
        for (int i = 0; i < sensor_count; i++) {
            if (strcmp(sensors[i].id, sensor_id) == 0) {
                sensor = &sensors[i];
                if (sensor->type != value_type) {
                    fprintf(stderr, "Warning: Type mismatch for sensor %s. Expected %d, got %d\n", 
                            sensor_id, sensor->type, value_type);
                    sensor = NULL;
                }
                break;
            }
        }
        
        if (!sensor) {
            if (sensor_count >= MAX_SENSORS) {
                fprintf(stderr, "Error: Too many sensors. Increase MAX_SENSORS.\n");
                continue;
            }
            sensor = &sensors[sensor_count++];
            strncpy(sensor->id, sensor_id, MAX_SENSOR_ID_LENGTH-1);
            sensor->id[MAX_SENSOR_ID_LENGTH-1] = '\0';
            sensor->type = value_type;
            sensor->count = 0;
            sensor->capacity = 0;
            sensor->readings = NULL;
        }
        
        add_reading(sensor, timestamp, value);
    }
    fclose(file);
    
    
#ifdef _WIN32
    if (_mkdir("sensor_data") != 0 && errno != EEXIST) {
#else
    if (mkdir("sensor_data", 0755) != 0 && errno != EEXIST) {
#endif
        perror("Error creating sensor_data directory");
        exit(EXIT_FAILURE);
    }
    
    
    for (int i = 0; i < sensor_count; i++) {
        SensorData *s = &sensors[i];
        
        
        qsort(s->readings, s->count, sizeof(SensorReading), compare_readings);
        
        
        char out_filename[256];
        snprintf(out_filename, sizeof(out_filename), "sensor_data/%s.dat", s->id);
        
        
        FILE *out = fopen(out_filename, "w");
        if (!out) {
            perror("Error opening output file");
            free(s->readings);
            continue;
        }
        
        for (int j = 0; j < s->count; j++) {
            SensorReading *r = &s->readings[j];
            fprintf(out, "%ld %s\n", r->timestamp, r->value_str);
        }
        
        if (fclose(out) != 0) {
            perror("Error closing output file");
        }
        
        printf("Created file for sensor %s with %d readings\n", s->id, s->count);
        
        
        free(s->readings);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    process_file(argv[1]);
    return EXIT_SUCCESS;
}