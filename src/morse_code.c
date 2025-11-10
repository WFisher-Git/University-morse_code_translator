// RUNGS AND LADDERS

// libraries and modules
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "includes/stdlib.h"
#include "includes/seven_segment.h"
#include "includes/buzzer.h"
#include "includes/potentiometer.h"
#include "buzzer.c"
#include "potentiometer.c"
#include "led.c"


// declare functions
void decoder();
int checkButton();
char last_char(char output[]);
void playNote(unsigned int frequency, unsigned int time);
void playSong();

// declare global variables
char decoded_message[5];
static unsigned int song[] = {A5, C6, C6, C6, D6, C6, D6, C6, GAP, A5, C6, F6, G6, G6, A5, G6, F6};
bool program_continue = true;

// make out own data structure called Dictionary in the style of a 2d array
// allows us to say .morse and .letter instead of [0][0] or [0][1]
typedef struct {
    char* morse;      
    char letter;    
} Dictionary;

// make dictionary using this new data structure
Dictionary dictionary[] = {
    {".-", 'A'},    {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'},
    {".", 'E'},     {"..-.", 'F'}, {"--.", 'G'},  {"....", 'H'},
    {"..", 'I'},    {".---", 'J'}, {"-.-", 'K'},  {".-..", 'L'},
    {"--", 'M'},    {"-.", 'N'},   {"---", 'O'},  {".--.", 'P'},
    {"--.-", 'Q'},  {".-.", 'R'},  {"...", 'S'},  {"-", 'T'},
    {"..-", 'U'},   {"...-", 'V'}, {".--", 'W'},  {"-..-", 'X'},
    {"-.--", 'Y'},  {"--..", 'Z'}
};

int main() {

	stdio_init_all();

	// initialise the button
	#define BUTTON_PIN			16	// Pin 21
	#define BUTTON_PIN2         22  // Pin 29
	gpio_init(BUTTON_PIN);
	gpio_set_dir(BUTTON_PIN, GPIO_IN);
	gpio_pull_down(BUTTON_PIN);
	gpio_init(BUTTON_PIN2);
	gpio_set_dir(BUTTON_PIN2, GPIO_IN);
	gpio_pull_down(BUTTON_PIN2);


	// Initialise the seven segment display
	seven_segment_init();
	seven_segment_off();

	// initialise the potentiometer
	potentiometer_init();

	// initialise rgb led
	setup_rgb();
	show_rgb(0,0);

	// 7 segment test 
	seven_segment_show(10);
	sleep_ms(200);
	

	while (true) {
		// declare local variables
		char output[99] = "";
		int pressed_time = 0;
		uint32_t lastPress = 0; 
		uint32_t time_since_last_press = 0;

		static unsigned int defaultTimeLimit = 100;
		static unsigned int potentiometer_limit = 4;
		unsigned int potentiometer_value = 2;
		unsigned int timeLimit = 0;
		unsigned int dotDashLimit = 0;

		printf("MORSE CODE TRANSLATOR\n---------------------\n");

		while (true) {
			
			pressed_time = checkButton();
			timeLimit = defaultTimeLimit * potentiometer_value;
			dotDashLimit = timeLimit/2;

			if (potentiometer_value != potentiometer_read(potentiometer_limit)) {
				potentiometer_value = potentiometer_read(potentiometer_limit);
				printf("New time limit to input: %d\n", potentiometer_value * defaultTimeLimit);
			}
			
			if (pressed_time != -1) { // button has been pressed
				lastPress = clock();

				// add either a dot or a dash depending on how long the press was
				if (pressed_time > 700) {
					printf("Invalid input, please try again...\n");
					playNote(150, 250);
					show_rgb(255,0);
					seven_segment_show(10);
				} else if (pressed_time < dotDashLimit) {
					strcat(output, "."); 
					printf(".");
					playNote(500, 250); // (frequency, duration)

				} else if (pressed_time >= dotDashLimit ) {
					strcat(output, "-"); 
					printf("-");
					playNote(500, 600); // (frequency, duration)
				}

			} else { // button has not been pressed
				time_since_last_press = (clock() - lastPress);

				// check if need to add a space
				if (strlen(output) > 0 && time_since_last_press > timeLimit && last_char(output) != ' ') {

					decoder(output);
					strcat(output, " "); 
					printf(" ");
					printf("\nDecoded message:  %s\n", decoded_message);

					if (strlen(decoded_message) >= 4) {
						printf("\n\nFinal decoded message:  %s\n", decoded_message);
						playSong();
						break;
					}
				}
			}
		}

		printf("\nWould you like to restart the program? (y/n)\n");

		while (true) {
			if (gpio_get(BUTTON_PIN)){
				// resets global variables and re-runs main
				memset(decoded_message, '\0', sizeof(decoded_message));
				show_rgb(0,255);
				printf("\nRestarting program...\n\n\n\n");
				sleep_ms(600);
				break;
			} else if (gpio_get(BUTTON_PIN2)) {
				// ends program
				show_rgb(255,0);
				printf("\nEnd of Program\n--------------\n");
				return 0;
			}
		}	
	}
	return 0;
}


// remove last character
char last_char(char output[]){
	return output[strlen(output)-1];
}

// translates input to morse
void decoder(char* output) {
	// sets a pointer at the last space (so we know everything after is the last input)
    char* last_input = strrchr(output, ' ');

	// if there is a space, the start of the next input is just after the space. so we move the pointer forward by 1
    if (last_input != NULL) {
        last_input++;  
    } else {
        // if no space is found, this is the first input. so we set the pointer to the start of the string
        last_input = output;
    }

	// used to check if we need to display an error in the case no translation is found
    int found = 0;

    for (int i = 0; i < 26; i++) {
		// check if the morse matches the morse in dictionary
        if (strcmp(dictionary[i].morse, last_input) == 0) {
			// add the found letter to the decoded message 
			decoded_message[strlen(decoded_message)] = dictionary[i].letter;
			seven_segment_show(i+11);
			show_rgb(0,255);

			// stop searching once we have found the match
            found = 1;
            break;  
        }
    }
    if (!found) {
        printf(":   No matching Morse code\n");
		show_rgb(255, 0);
		playNote(150, 250);
		seven_segment_show(10);
    }
}

// returns how long the button has been pressed for
int checkButton(){

	if (gpio_get(BUTTON_PIN)){ // button pressed
		uint32_t start_button_time = clock();

		while (gpio_get(BUTTON_PIN)){} // do nothing while button is down

		// once released, work out how long the button was down
		uint32_t end_button_time = clock();
		int pressed_time = (end_button_time-start_button_time);
		return pressed_time;

	} else { // button not pressed
		return -1;
	} 
}

// plays a note
void playNote(unsigned int frequency, unsigned int time) {
	buzzer_init();
	buzzer_enable(frequency);
	sleep_ms(time);
	buzzer_disable();
	sleep_ms(30);
}

// hardcoded H-O-T-T-O-G-O. LEAVE THIS ALONE...
void playSong() {

	unsigned int time;

	for (int i = 0; i < 17; i++) {

		switch (i) {
			case 0: time = 320; break;
			case 1: time = 300; break;
			case 2: time = 300; break;
			case 3: time = 300; break;
			case 4: time = 400; break;
			case 5: time = 200; break;
			case 6: time = 320; break;
			case 7: time = 200; break;
			case 8: time = 0; sleep_ms(150); break;
			case 9: time = 320; break;
			case 10: time = 300; break;
			case 11: time = 300; break;
			case 12: time = 300; break;
			case 13: time = 400; break;
			case 14: time = 200; break;
			case 15: time = 320; break;
			case 16: time = 200; break;
			default: time = 0;
			}
			
		playNote(song[i], time);
	}
}
