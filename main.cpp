#include "mbed.h"
#include "LM75B.h"
#include "C12832.h"
#include "LocalFileSystem.h"
#include "Thread.h" // added Thread.h header file

#define LOG_INTERVAL_MINUTES 5

C12832 lcd(p5, p7, p6, p8, p11);
LM75B sensor(p28, p27);
LocalFileSystem local("local");

void write_to_file_thread() {
    FILE *fp = fopen("/local/temp_log.txt", "a");
    if (fp == NULL) {
        printf("Could not open file for appending\n");
        return;
    }
    
    while (1) {
        float temp = sensor.read();
        fprintf(fp, "%.3f\n", temp);
        ThisThread::sleep_for(1.0);
    }
}

void update_display_thread() {
    FILE *fp = fopen("/local/temp_log.txt", "r");
    if (fp == NULL) {
        printf("Could not open file for reading\n");
        return;
    }
    
    float min_t = 100;
    float max_t = -100;
    float avg_t = 0.0;
    int num_samples = 0;

    while (1) {
        float sum_t = 0.0;
        num_samples = 0;
        
        fseek(fp, 0, SEEK_SET); 
        
        while (!feof(fp)) {
            float temp = 0.0;
            fscanf(fp, "%f\n", &temp);
            sum_t += temp;
            if (temp < min_t) {
                min_t = temp;
            }
            if (temp > max_t) {
                max_t = temp;
            }
            num_samples++;
        }
        avg_t = sum_t / num_samples;

        lcd.cls();
        lcd.locate(0, 3);
        lcd.printf("Min: %.3f\nMax: %.3f\nAvg: %.3f", min_t, max_t, avg_t);

        ThisThread::sleep_for(LOG_INTERVAL_MINUTES * 60);
    }
}

int main() {
    Thread write_thread;
    Thread display_thread;

    write_thread.start(callback(write_to_file_thread)); // use callback to start thread function
    display_thread.start(callback(update_display_thread)); // use callback to start thread function
    
    while (1) {
        // do nothing, allow threads to run
    }
}
