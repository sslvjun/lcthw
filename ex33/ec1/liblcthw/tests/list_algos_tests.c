#include "minunit.h"
#include <lcthw/list_algos.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>

char *values[] = {"XXXX", "1234", "abcd", "xjvef", "NDSS", "I", "like", "kopro", "experiments"};
#define NUM_VALUES 9

#define SUBLIST_MIN_SIZE 1

#define BILLION 1000000000UL

#define ITER 1000000L

List *create_words()
{
	int i = 0;
	List *words = List_create();
	
	for(i = 0; i < NUM_VALUES; i++) {
		List_push(words, values[i]);
	}

	return words;
}

long get_diff(struct timespec start, struct timespec end)
{
	return (unsigned long)(end.tv_sec - start.tv_sec) * BILLION + (unsigned long)(end.tv_nsec - start.tv_nsec);
}

void stack_increase()
{
	const rlim_t kStackSize = 256L * 1024L * 1024L;
	struct rlimit rl;
	int result;

	result = getrlimit(RLIMIT_STACK, &rl);
	if(result == 0) {
		if(rl.rlim_cur < kStackSize) {	
			rl.rlim_cur = kStackSize;

			result = setrlimit(RLIMIT_STACK, &rl);

            if(result != 0) {
                fprintf(stderr, "setrlimit returned result = %d\n", result);
			}
		}
	}
}

char *test_bubble_sort()
{
	List *words = create_words();

	// should work on a list that needs sorting
	int rc = List_bubble_sort(words, (List_compare)strcmp);
	mu_assert(rc == 0, "Bubble sort failed.");
	mu_assert(check_sorting(words, (List_compare)strcmp), "Words are not sorted after bubble sort.");

	// should work on already sorted list
	rc = List_bubble_sort(words, (List_compare)strcmp);
	mu_assert(rc == 0, "Bubble sort of already sorted failed.");
	mu_assert(check_sorting(words, (List_compare)strcmp), "Words should be sort if already bubble sorted.");

	List_clear_destroy(words);

	// should work on an empty list
	words = List_create(words);
	rc = List_bubble_sort(words, (List_compare)strcmp);
	mu_assert(rc == 0, "Bubble sort failed on empty list.");
	mu_assert(check_sorting(words, (List_compare)strcmp), "Words should be sorted if empty.");

	List_clear_destroy(words);

	return NULL;
}

char *test_merge_sort()
{
	List *words = create_words();

	// should work on a list that needs sorting
	List *res = List_merge_sort(words, (List_compare)strcmp, SUBLIST_MIN_SIZE);
	mu_assert(check_sorting(res, (List_compare)strcmp), "Words are not sorted after merge sort.");

	List *res2 = List_merge_sort(res, (List_compare)strcmp, SUBLIST_MIN_SIZE);
	mu_assert(check_sorting(res, (List_compare)strcmp), "Should still be sorted after merge sort.");

	if(res2 != res) List_clear_destroy(res2);
	if(res != words) List_clear_destroy(res);

	List_clear_destroy(words);

	return NULL;
}

char *test_insert_sort()
{
	List *words = create_words();

	// should work on a list that needs sorting
	List *res = List_insert_sorted(words, (List_compare)strcmp);
	mu_assert(check_sorting(res, (List_compare)strcmp), "Words are not sorted after insert sort.");
	mu_assert(List_count(res) == NUM_VALUES, "Wrong count of nodes on insert sort.");

	List *res2 = List_insert_sorted(res, (List_compare)strcmp);
	mu_assert(check_sorting(res, (List_compare)strcmp), "Should still be sorted after insert sort.");

	if(res2 != res) List_clear_destroy(res2);
	if(res != words) List_clear_destroy(res);

	List_clear_destroy(words);

	return NULL;
}

char *test_bottom_up_sort()
{
	List *words = create_words();

	// should work on a list that needs sorting
	List *res = List_bottom_up_sort(words, (List_compare)strcmp);
	mu_assert(check_sorting(res, (List_compare)strcmp), "Words are not sorted after bottom up sort.");
	mu_assert(List_count(res) == NUM_VALUES, "Wrong count of nodes on bottom up sort.");

	List *res2 = List_bottom_up_sort(res, (List_compare)strcmp);
	mu_assert(check_sorting(res, (List_compare)strcmp), "Should still be sorted after bottom up sort.");
	mu_assert(List_count(res) == NUM_VALUES, "Wrong count of nodes on bottom up sort.");

	if(res2 != res) List_clear_destroy(res2);
	if(res != words) List_clear_destroy(res);

	List_clear_destroy(words);
	
	return NULL;
}

char *test_bubble_perfomance()
{
	struct timespec start, end;
	double diff;

	int i = 0;

	List *bubble_words[ITER];
	int rc[ITER];

	// bubble sort bootstrap
	for(i = 0; i < ITER; i++) {
		bubble_words[i] = create_words();
	}

	// bubble sort measuring
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for(i = 0; i < ITER; i++) {
		rc[i] = List_bubble_sort(bubble_words[i], (List_compare)strcmp);
	}

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	diff = (double)get_diff(start, end) / ITER;
	printf("\nBubble sort took %lf nanoseconds to run.\n", diff);

	// bubble sort checking results and freeing of resources
	/*
	for(i = 0; i < ITER; i++) {
		mu_assert(rc[i] == 0, "Bubble sort failed.");
		mu_assert(check_sorting(bubble_words[i], (List_compare)strcmp), "Words are not sorted after bubble sort.");
		List_clear_destroy(bubble_words[i]);
	}
	*/
	return NULL;
}

char *test_merge_perfomance()
{
	struct timespec start, end;
	double diff;

	int i = 0;

	List *merge_words = NULL;
	List *merged_words[ITER];

	// merge sort bootstrap
	merge_words = create_words();
	
	// merge sort measuring
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for(i = 0; i < ITER; i++) {
		merged_words[i] = List_merge_sort(merge_words, (List_compare)strcmp, SUBLIST_MIN_SIZE);
	}

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	diff = (double)get_diff(start, end) / ITER;
	printf("\nMerge sort took %lf nanoseconds to run.\n", diff);

	// merge sort checking results and freeing of resources
	/*
	List_clear_destroy(merge_words);
	for(i = 0; i < ITER; i++) {
		mu_assert(check_sorting(merged_words[i], (List_compare)strcmp), "Words are not sorted after merge sort.");
		List_clear_destroy(merged_words[i]);
	}
	*/

	return NULL;
}

char *test_insert_perfomance()
{
	struct timespec start, end;
	double diff;

	int i = 0;

	List *insert_words[ITER];
	List *insert_sorted_words[ITER];

	// insert sort bootstrap
	for(i = 0; i < ITER; i++) {
		insert_words[i] = create_words();
	}

	// insert sort measuring
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for(i = 0; i < ITER; i++) {
		insert_sorted_words[i] = List_insert_sorted(insert_words[i], (List_compare)strcmp);
	}

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	diff = (double)get_diff(start, end) / ITER;
	printf("\nInsert sort took %lf nanoseconds to run.\n", diff);

	// insert sort checking results and freeing of resources
	/*
	for(i = 0; i < ITER; i++) {
		mu_assert(check_sorting(insert_sorted_words[i], (List_compare)strcmp), "Words are not sorted after insert sort.");
		List_clear_destroy(insert_sorted_words[i]);
		List_clear_destroy(insert_words[i]);
	}
	*/

	return NULL;
}

char *test_bottom_up_perfomance()
{
	struct timespec start, end;
	double diff;

	int i = 0;

	List *bottom_up_words[ITER];
	List *bottom_up_sorted_words[ITER];

	// bottom up sort bootstrap
	for(i = 0; i < ITER; i++) {
		bottom_up_words[i] = create_words();
	}

	// bottom up sort measuring
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for(i = 0; i < ITER; i++) {
		bottom_up_sorted_words[i] = List_bottom_up_sort(bottom_up_words[i], (List_compare)strcmp);
	}

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	diff = (double)get_diff(start, end) / ITER;
	printf("\nBottom up sort took %lf nanoseconds to run.\n", diff);

	// bottom up sort checking results and freeing of resources
	/*
	for(i = 0; i < ITER; i++) {
		mu_assert(check_sorting(bottom_up_sorted_words[i], (List_compare)strcmp), "Words are not sorted after bottom up sort.");
		List_clear_destroy(bottom_up_sorted_words[i]);
		List_clear_destroy(bottom_up_words[i]);
	}
	*/

	return NULL;
}

char *all_tests()
{
	mu_suite_start();

	mu_run_test(test_bubble_sort);
	mu_run_test(test_merge_sort);
	mu_run_test(test_insert_sort);
	mu_run_test(test_bottom_up_sort);

	stack_increase();

	mu_run_test(test_bubble_perfomance);
	mu_run_test(test_merge_perfomance);
	mu_run_test(test_insert_perfomance);
	mu_run_test(test_bottom_up_perfomance);

	return NULL;
}

RUN_TESTS(all_tests);
