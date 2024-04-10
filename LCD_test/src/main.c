#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "esp_adc_cal.h"
#include "sdkconfig.h"
#include "HD44780.h"
#include "math.h"
#include "string.h"


#define LCD_ADDR 0x27
#define SDA_PIN  21
#define SCL_PIN  22
#define LCD_COLS 16
#define LCD_ROWS 2
#define DEFAULT_VREF    1100        // Default voltage reference (in mV) for ADC
#define ADC_WIDTH       ADC_WIDTH_BIT_12
#define ADC_CHANNEL_THUMB     ADC_CHANNEL_0  // GPIO0
#define ADC_CHANNEL_INDEX     ADC_CHANNEL_1  // GPIO1
#define ADC_CHANNEL_MIDDLE     ADC_CHANNEL_2  // GPIO2
#define ADC_CHANNEL_RING     ADC_CHANNEL_3  // GPIO3
#define ADC_CHANNEL_PINKY     ADC_CHANNEL_4  // GPIO4

typedef union {
    float  v1;
    float  v2;
    float  v3;
    float  v4;
    float  v5;
    // Add more data types as needed
} LetterDataType;

int closest_letter(float thumb_voltage,float index_voltage,float middle_voltage,float ring_voltage,float pinky_voltage, LetterDataType *letter_array){
    float min_distance = 10000;
    int min_character = -1;
    LetterDataType current_letter;
    float curr_distance = 0.0;
    for (int i = 0; i < 2; i++) {
        curr_distance = 0;
        current_letter = letter_array[i];
        curr_distance = curr_distance + pow(current_letter.v1 - thumb_voltage, 2);
        curr_distance = curr_distance + pow(current_letter.v2 - index_voltage, 2);
        curr_distance = curr_distance + pow(current_letter.v3 - middle_voltage, 2);
        curr_distance = curr_distance + pow(current_letter.v4 - ring_voltage, 2);
        curr_distance = curr_distance + pow(current_letter.v5 - pinky_voltage, 2);
        curr_distance = sqrt(curr_distance);

        if(curr_distance < min_distance){
            min_distance = curr_distance;
            min_character = i;
            //strcpy(min_character, current_letter.letter);
        }
        
    }
    //min_character = 1;
    return min_character;
}

char* char_to_string(char c) {
    // Allocate memory for the string (character array)
    char* str = (char*)malloc(2 * sizeof(char)); // Allocate memory for one character and null terminator
    if (str == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    // Copy the character into the string
    str[0] = c;
    str[1] = '\0'; // Null terminator

    return str;
}


void LCD_DemoTask(void* param)
{
    char curr_str[20] = "CURR STR";
    char prev_str[20] = "PREV STR";
    float voltage_1 = 0.0;
    float voltage_2 = 0.0;
    float voltage_3 = 0.0;
    float voltage_4 = 0.0;
    float voltage_5 = 0.0;
    char *letter_array[] = {"A", "B"};
    LetterDataType ideal_array[26];
    ideal_array[0].v1 = 2.6;
    ideal_array[0].v2 = 2.5;
    ideal_array[0].v3 = 2.5;
    ideal_array[0].v4 = 2.4;
    ideal_array[0].v5 = 2.5;

    ideal_array[1].v1 = 2.5;
    ideal_array[1].v2 = 0.1;
    ideal_array[1].v3 = 0.1;
    ideal_array[1].v4 = 0.1;
    ideal_array[1].v5 = 0.1;


    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL_THUMB, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC_CHANNEL_INDEX, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC_CHANNEL_MIDDLE, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC_CHANNEL_RING, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC_CHANNEL_PINKY, ADC_ATTEN_DB_11);

    while (true) {

        LCD_home();
        LCD_clearScreen();

        voltage_1 = adc1_get_raw(ADC_CHANNEL_THUMB);
        voltage_2 = adc1_get_raw(ADC_CHANNEL_INDEX);
        voltage_3 = adc1_get_raw(ADC_CHANNEL_MIDDLE);
        voltage_4 = adc1_get_raw(ADC_CHANNEL_RING);
        voltage_5 = adc1_get_raw(ADC_CHANNEL_PINKY);
        /*voltage_1 = 0.5;
        voltage_2 = 0.5;
        voltage_3 = 0.5;
        voltage_4 = 0.5;
        voltage_5 = 0.5;*/

        int new_letter_pos = closest_letter(voltage_1, voltage_2, voltage_3, voltage_4, voltage_5, ideal_array);
        char* new_letter = (char *)malloc(3);
        new_letter = letter_array[new_letter_pos];
        //strcpy(new_letter, ideal_array[new_letter_pos].letter);
        //new_letter = "A";
        if(strlen(curr_str) == 16){
           strcpy(prev_str, curr_str);
           strcpy(curr_str, new_letter);
        }
        else if(strlen(curr_str) < 16)
        {
           strcat(curr_str, new_letter);
          //strcpy(curr_str, curr_str + new_letter);
        }
        //strcpy(prev_str, new_str);
        //strcpy(curr_str, new_str);
        LCD_setCursor(0,0);
        LCD_writeStr(prev_str);
        LCD_setCursor(0,1);
        LCD_writeStr(curr_str);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}
void app_main()
{
    LCD_init(LCD_ADDR, SDA_PIN, SCL_PIN, LCD_COLS, LCD_ROWS);
    xTaskCreate(&LCD_DemoTask, "Demo Task", 2048, NULL, 5, NULL);
}
