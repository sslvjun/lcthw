#include <lcthw/string_algos.h>
#include <limits.h>

static inline void String_setup_skip_chars(
		size_t *skip_chars, 
		const unsigned char *needle, ssize_t nlen)
{
	size_t i = 0;
	size_t last = nlen - 1;

	for(i = 0; i < UCHAR_MAX + 1; i++) {
		skip_chars[i] = nlen;
	}

	for(i = 0; i < last; i++) {
		skip_chars[needle[i]] = last - i;
	}
}

static inline const unsigned char *String_base_search(
		const unsigned char *haystack, ssize_t hlen,
		const unsigned char *needle, ssize_t nlen,
		size_t *skip_chars)
{
	size_t i = 0;
	size_t last = nlen - 1;

	assert(haystack != NULL && "Given bad haystack to search.");
	assert(needle != NULL && "Given bad needle to search for.");
	
	check(nlen > 0, "nlen can't be <= 0");
	check(hlen > 0, "hlen can't be <= 0");

	while(hlen >= nlen) {
		for(i = last; haystack[i] == needle[i]; i--) {
			if(i == 0) {
				return haystack;
			}
		}

		hlen -= skip_chars[haystack[last]];
		haystack += skip_chars[haystack[last]];
	}

error: // fallthrough
	return NULL;
}

int String_find(bstring in, bstring what, int reset)
{
	static const unsigned char *prev_in = NULL;
	static const unsigned char *prev_what = NULL;

	const unsigned char *cur_in = (const unsigned char *)bdata(in);
	const unsigned char *cur_what = (const unsigned char *)bdata(what);

	const unsigned char *found = NULL;

	static const unsigned char *haystack = NULL;
	static ssize_t hlen = 0;
	
	const unsigned char *needle = cur_what;
	ssize_t nlen = blength(what);
	
	static size_t skip_chars[UCHAR_MAX + 1] = {0};

	static ssize_t found_at = 0;

	if(!prev_in && !prev_what) {
		prev_in = cur_in;
		prev_what = cur_what;
	}

	if(cur_in != prev_in || cur_what != prev_what) {
		reset = 1;
	}

	if(reset) {
		found_at = 0;

		haystack = cur_in;
		hlen = blength(in);

		String_setup_skip_chars(skip_chars, needle, nlen);
	}

	if(found_at != 0 && hlen <= 0) {
		haystack = cur_in;
		hlen = blength(in);

		return -1;
	}

	found = String_base_search(haystack, hlen, needle, nlen, skip_chars);

	if(found) {
		found_at = found - cur_in;
		haystack = found + nlen;
		hlen -= found_at - nlen;
	} else {
		found_at = -1;
	}

	return found_at;
}

StringScanner *StringScanner_create(bstring in)
{
	StringScanner *scan = calloc(1, sizeof(StringScanner));
	check_mem(scan);

	scan->in = in;
	scan->haystack = (const unsigned char *)bdata(in);
	scan->hlen = blength(in);

	assert(scan != NULL && "fuck");
	return scan;

error:
	free(scan);
	return NULL;
}

static inline void StringScanner_set_needle(StringScanner *scan, bstring tofind)
{
	scan->needle = (const unsigned char *)bdata(tofind);
	scan->nlen = blength(tofind);

	String_setup_skip_chars(scan->skip_chars, scan->needle, scan->nlen);
}

static inline void StringScanner_reset(StringScanner *scan)
{
	scan->haystack = (const unsigned char *)bdata(scan->in);
	scan->hlen = blength(scan->in);
}

int StringScanner_scan(StringScanner *scan, bstring tofind)
{
	const unsigned char *found = NULL;
	ssize_t found_at = 0;

	if(scan->hlen <= 0)	{
		StringScanner_reset(scan);
		return -1;
	}

	if((const unsigned char *)bdata(tofind) != scan->needle) {
		StringScanner_set_needle(scan, tofind);
	}

	found = String_base_search(
			scan->haystack, scan->hlen,
			scan->needle, scan->nlen,
			scan->skip_chars);

	if(found) {
		found_at = found - (const unsigned char *)bdata(scan->in);
		scan->haystack = found + scan->nlen;
		scan->hlen -= found_at - scan->nlen;
	} else {
		// done, reset the setup
		StringScanner_reset(scan);
		found_at = -1;
	}

	return found_at;
}

void StringScanner_destroy(StringScanner *scan)
{
	if(scan) {
		free(scan);
	}
}
