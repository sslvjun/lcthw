/** WARNING: This code fresh and potentially isn't correct yet. */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "dbg.h"

#define MAX_DATA 5

/*
#define MAX_DATA 100
*/

int read_string(char **out_string, int max_buffer)
{
	// Try also giving it more data than MAX_DATA, 
	// and then see how not using calloc in read_string changes how it works.
	
	/*	
	*out_string = calloc(1, max_buffer + 1);
	check_mem(*out_string);
	*/

	*out_string = calloc(1, max_buffer + 1);
	check_mem(*out_string);
	
	// Finally, there's a problem that fgets eats the newlines, 
	// so try to fix that using fgetc but leave out the \0 that ends the string.

	int i = 0;
	char c = '\0';
	
	for(i = 0, c = fgetc(stdin); 
		i < max_buffer && c != '\n'; 
		i++, c = fgetc(stdin)) {

		*(*out_string + i) = c;
	}

	return 0;

error:
	if(*out_string) free(*out_string);
	*out_string = NULL;
	return -1;
}

int read_int(int *out_int)
{
	char *input = NULL;
	int rc = read_string(&input, MAX_DATA);
	check(rc == 0, "Failed to read number.");

	*out_int = atoi(input);

	free(input);
	return 0;

error:
	if(input) free(input);
	return -1;
}

int read_scan(const char *fmt, ...)
{
	int i = 0;
	int rc = 0;
	int *out_int = NULL;
	char *out_char = NULL;
	char **out_string = NULL;
	int max_buffer = 0;
	
	va_list argp;
	va_start(argp, fmt);
	
	for(i = 0; fmt[i] != '\0'; i++) {
		if(fmt[i] == '%') {
			i++;
			switch(fmt[i]) {
				case '\0':
					sentinel("Invalid format, you ended with %%.");
					break;
				
				case 'd':
					out_int = va_arg(argp, int *);
					rc = read_int(out_int);
					check(rc == 0, "Failed to read int.");
					break;

				case 'c':
					out_char = va_arg(argp, char *);
					*out_char = fgetc(stdin);
					break;
				
				case 's':
					max_buffer = va_arg(argp, int);
					out_string = va_arg(argp, char **);
					rc = read_string(out_string, max_buffer);
					check(rc == 0, "Failed to read string.");
					break;
		
				default:
					sentinel("Invalid format.");
			}
		} else {
			fgetc(stdin);
		}
		
		check(!feof(stdin) && !ferror(stdin), "Input error.");
	}
	
	va_end(argp);
	return 0;

error:
	va_end(argp);
	return -1;
}

int main(int argc, char *argv[])
{
	char *first_name = NULL;
	char initial = ' ';
	char *last_name = NULL;
	int age = 0;
	
	// This program should be more robust against buffer overflows, 
	// but it doesn't handle the formatted input as well as scanf. 
	// To try breaking this, change the code that you forget to pass in the initial size for '%s' formats.
	
	/*
	printf("What's your first name? ");
	int rc = read_scan("%s", &first_name);
	check(rc == 0, "Failed first name.");
	*/

	printf("What's your first name? ");
	int rc = read_scan("%s", MAX_DATA, &first_name);
	check(rc == 0, "Failed first name.");

	printf("What's your initial? ");
	rc = read_scan("%c\n", &initial);
	check(rc == 0, "Failed initial.");

	printf("What's your last name? ");
	rc = read_scan("%s", MAX_DATA, &last_name);
	check(rc == 0, "Failed last name.");
	
	printf("How old are you? ");
	rc = read_scan("%d", &age);

	printf("---- RESULTS ----\n");
	printf("First Name: %s", first_name);
	printf("Initial: '%c'\n", initial);
	printf("Last Name: %s", last_name);
	printf("Age: %d\n", age);

	free(first_name);
	free(last_name);
	return 0;
error:
	return 1;
}