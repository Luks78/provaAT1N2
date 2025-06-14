#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_LINE_LENGTH 256
#define MAX_SENSOR_ID_LENGTH 50

typedef struct {
    long timestamp;
    char value[MAX_LINE_LENGTH];
} SensorReading;

int load_sensor_data(const char *sensor_id, SensorReading **data) {
    char filename[MAX_SENSOR_ID_LENGTH + 20];
    snprintf(filename, sizeof(filename), "sensor_data/%s.dat", sensor_id);
    
    FILE *file = fopen(filename, "r");
    if (!file) {
        return -1; 
    }

     
    int count = 0;
    char buffer[MAX_LINE_LENGTH];
    while (fgets(buffer, sizeof(buffer), file)) count++;
    rewind(file);

    *data = malloc(count * sizeof(SensorReading));
    if (!*data) {
        fclose(file);
        return -2; 
    }

    
    int i = 0;
    while (fgets(buffer, sizeof(buffer), file) && i < count) {
        sscanf(buffer, "%ld %255[^\n]", &(*data)[i].timestamp, (*data)[i].value);
        i++;
    }

    fclose(file);
    return i; 
}

int binary_search_closest(SensorReading *data, int count, long target) {
    int left = 0, right = count - 1;
    int closest = 0;
    long min_diff = LONG_MAX;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        long diff = labs(data[mid].timestamp - target);

        if (diff < min_diff) {
            min_diff = diff;
            closest = mid;
        }

        if (data[mid].timestamp < target) {
            left = mid + 1;
        } else if (data[mid].timestamp > target) {
            right = mid - 1;
        } else {
            
            return mid;
        }
    }

    
    if (closest > 0 && 
        labs(data[closest-1].timestamp - target) < min_diff) {
        closest--;
    }
    if (closest < count-1 && 
        labs(data[closest+1].timestamp - target) < min_diff) {
        closest++;
    }

    return closest;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <sensor_id> <timestamp>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s sensor1 1625097600\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *sensor_id = argv[1];
    long target_timestamp = atol(argv[2]);

    SensorReading *sensor_data = NULL;
    int count = load_sensor_data(sensor_id, &sensor_data);

    if (count <= 0) {
        if (count == -1) {
            fprintf(stderr, "Erro: Sensor '%s' não encontrado.\n", sensor_id);
        } else {
            fprintf(stderr, "Erro: Falha ao carregar dados do sensor.\n");
        }
        return EXIT_FAILURE;
    }

    int closest_index = binary_search_closest(sensor_data, count, target_timestamp);
    SensorReading *result = &sensor_data[closest_index];

    printf("\nLeitura mais proxima encontrada:\n");
    printf("--------------------------------\n");
    printf("Sensor: %s\n", sensor_id);
    printf("Timestamp: %ld\n", result->timestamp);
    printf("Valor: %s\n", result->value);
    printf("Diferenca: %ld segundos\n", labs(result->timestamp - target_timestamp));
    printf("--------------------------------\n");

    free(sensor_data);
    return EXIT_SUCCESS;
}