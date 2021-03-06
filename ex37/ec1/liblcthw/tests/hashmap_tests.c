#include "minunit.h"
#include <lcthw/hashmap.h>
#include <assert.h>
#include <lcthw/bstrlib.h>
#include <lcthw/perfomance_routines.h>

Hashmap *map = NULL;
static int traverse_called = 0;
struct tagbstring test1 = bsStatic("test data 1");
struct tagbstring test2 = bsStatic("test data 2");
struct tagbstring test3 = bsStatic("test data 3");
struct tagbstring expect1 = bsStatic("THE VALUE 1");
struct tagbstring expect2 = bsStatic("THE VALUE 2");
struct tagbstring expect3 = bsStatic("THE VALUE 3");

static uint32_t djb2_hash(void *a)
{
	char *key = bdata((bstring)a);

	uint32_t hash = 5381;
	int c;

	while(c = *key++) {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}

static int nodes_distribution(Hashmap *map)
{
	int i = 0;

	debug("NODES DISTRIBUTION:");
	for(i = 0; i < DArray_count(map->buckets); i++) {
		DArray *bucket = DArray_get(map->buckets, i);
		if(bucket) {
			debug("BUCKET %d:\t %d nodes", i, DArray_count(bucket));
		}
	}

	return 0;
}

static int traverse_good_cb(HashmapNode *node)
{
	debug("KEY: %s", bdata((bstring)node->key));
	traverse_called++;
	return 0;
}

static int traverse_fail_cb(HashmapNode *node)
{
	debug("KEY: %s", bdata((bstring)node->key));
	traverse_called++;

	if(traverse_called == 2) {
		return 1;
	} else {
		return 0;
	}
}

char *test_create()
{
	map = Hashmap_create(NULL, NULL);
	mu_assert(map != NULL, "Failed to create map.");

	return NULL;
}

char *test_destroy()
{
	Hashmap_destroy(map);

	return NULL;
}

char *test_get_set()
{
	int rc = Hashmap_set(map, &test1, &expect1);
	mu_assert(rc == 0, "Failed to set &test1");
	bstring result = Hashmap_get(map, &test1);
	mu_assert(result == &expect1, "Wrong value for test1.");

	rc = Hashmap_set(map, &test2, &expect2);
	mu_assert(rc == 0, "Failed to set test2");
	result = Hashmap_get(map, &test2);
	mu_assert(result == &expect2, "Wrong value for test2");

	rc = Hashmap_set(map, &test3, &expect3);
	mu_assert(rc == 0, "Failed to set test3.");
	result = Hashmap_get(map, &test3);
	mu_assert(result == &expect3, "Wrong value for test3.");

	nodes_distribution(map);

	return NULL;
}

char *test_traverse()
{
	int rc = Hashmap_traverse(map, traverse_good_cb);
	mu_assert(rc == 0, "Failed to traverse.");
	mu_assert(traverse_called == 3, "Wrong count traverse.");

	traverse_called = 0;
	rc = Hashmap_traverse(map, traverse_fail_cb);
	mu_assert(rc == 1, "Failed to traverse.");
	mu_assert(traverse_called == 2, "Wrong count traverse for fail.");

	return NULL;
}

char *test_delete()
{
	bstring deleted = (bstring)Hashmap_delete(map, &test1);
	mu_assert(deleted != NULL, "Got NULL on delete.");
	mu_assert(deleted == &expect1, "Should get test1");
	bstring result = Hashmap_get(map, &test1);
	mu_assert(result == NULL, "Should delete.");

	deleted = (bstring)Hashmap_delete(map, &test2);
	mu_assert(deleted != NULL, "Got NULL on delete.");
	mu_assert(deleted == &expect2, "Should get test2");
	result = Hashmap_get(map, &test2);
	mu_assert(result == NULL, "Should delete.");

	deleted	= (bstring)Hashmap_delete(map, &test3);
	mu_assert(deleted != NULL, "Got NULL on delete.");
	mu_assert(deleted == &expect3, "Should get test3");
	result = Hashmap_get(map, &test3);
	mu_assert(result == NULL, "Should delete.");
	
	return NULL;
}

char *test_rehash()
{
	Hashmap *map1 = Hashmap_create_advanced(NULL, djb2_hash, 1, 1);

	mu_assert(map1->buckets_number == 1, "Wrong buckets number.");
	
	// set 1
	Hashmap_set(map1, &test1, &expect1);

	traverse_called = 0;
	Hashmap_traverse(map1, traverse_good_cb);
	mu_assert(traverse_called == 1, "Wrong count traverse.");	
	mu_assert(map1->buckets_number == 1, "Wrong buckets number.");
	nodes_distribution(map1);
	debug("_______________________________________________");

	// set 2
	Hashmap_set(map1, &test2, &expect2);

	traverse_called = 0;
	Hashmap_traverse(map1, traverse_good_cb);
	mu_assert(traverse_called == 2, "Wrong count traverse.");
	mu_assert(map1->buckets_number == 2, "Wrong buckets number.");
	nodes_distribution(map1);
	debug("_______________________________________________");

	// set 3
	Hashmap_set(map1, &test3, &expect3);

	traverse_called = 0;
	Hashmap_traverse(map1, traverse_good_cb);
	mu_assert(traverse_called == 3, "Wrong count traverse.");
	mu_assert(map1->buckets_number == 3, "Wrong buckets number.");
	nodes_distribution(map1);
	debug("_______________________________________________");

	// delete 1
	Hashmap_delete(map1, &test1);

	traverse_called = 0;
	Hashmap_traverse(map1, traverse_good_cb);
	mu_assert(traverse_called == 2, "Wrong count traverse.");
	mu_assert(map1->buckets_number == 2, "Wrong buckets number.");
	nodes_distribution(map1);
	debug("_______________________________________________");

	// delete 2
	Hashmap_delete(map1, &test2);

	traverse_called = 0;
	Hashmap_traverse(map1, traverse_good_cb);
	mu_assert(traverse_called == 1, "Wrong count traverse.");
	mu_assert(map1->buckets_number == 1, "Wrong buckets number.");
	nodes_distribution(map1);
	debug("_______________________________________________");
	
	// delete 3
	Hashmap_delete(map1, &test3);

	traverse_called = 0;
	Hashmap_traverse(map1, traverse_good_cb);
	mu_assert(traverse_called == 0, "Wrong count traverse.");
	mu_assert(map1->buckets_number == 1, "Wrong buckets number.");
	nodes_distribution(map1);
	debug("_______________________________________________");
	
	// destroy
	Hashmap_destroy(map1);

	return NULL;
}

static inline bstring generate_string()
{
	int str_len = rand() % 99 + 1 + 1;
	char *str = malloc(str_len);

	int i = 0;

	for(i = 0; i < str_len - 1; i++) {
		*(str + i) = '0' + rand() % 72 + 1;
	}
	*(str + i) = '\0';

	bstring bstr = bfromcstr(str);

	free(str);

	return bstr;
}

#define STRINGS_NUMBER 10

char *filling_defect()
{
	// part 0
	bstring keys[STRINGS_NUMBER] = {NULL};
	bstring values[STRINGS_NUMBER] = {NULL};
	bstring result = NULL;

	int i = 0;

	for(i = 0; i < STRINGS_NUMBER; i++) {
		keys[i] = generate_string();
		values[i] = generate_string();
	}
	
	Hashmap *map1 = Hashmap_create_advanced(NULL, djb2_hash, 1, 1);	

	// part 1
	debug("PART 1");
	for(i = 0; i < STRINGS_NUMBER; i++) {
		Hashmap_set(map1, keys[i], values[i]);
		mu_assert(map1->buckets_number == i + 1, "Wrong buckets number.");

		result = Hashmap_get(map1, keys[i]);
		mu_assert(result == values[i], "Wrong value.");
	}
	debug("NUMBER OF NODES: %d", map1->counter);
	mu_assert(map1->counter == STRINGS_NUMBER, "Wrong number of nodes.");
	nodes_distribution(map1);

	// part 2
	debug("PART 2");
	for(i = 0; i < STRINGS_NUMBER; i++) {
		result = Hashmap_get(map1, keys[i]);
		mu_assert(result == values[i], "Wrong value.");
	}

	// part 3
	debug("PART 3");
	for(i = 0; i < STRINGS_NUMBER / 2; i++) {
		Hashmap_delete(map1, keys[i]);
		mu_assert(map1->buckets_number == STRINGS_NUMBER - i - 1, "Wrong buckets number.");
	}
	debug("NUMBER OF NODES: %d", map1->counter);
	mu_assert(map1->counter == STRINGS_NUMBER / 2, "Wrong number of nodes.");
	nodes_distribution(map1);

	// part 4
	Hashmap_destroy(map1);
	
	for(i = 0; i < STRINGS_NUMBER; i++) {
		bdestroy(keys[i]);
		bdestroy(values[i]);
	}

	return NULL;
}

char *all_tests()
{
	mu_suite_start();

	mu_run_test(test_create);
	mu_run_test(test_get_set);
	mu_run_test(test_traverse);
	mu_run_test(test_delete);
	mu_run_test(test_destroy);
	
	mu_run_test(test_rehash);

	srand(time(NULL));
	mu_run_test(filling_defect);

	return NULL;
}

RUN_TESTS(all_tests);
