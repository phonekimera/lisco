#include <stdio.h>
#include <stdlib.h>

#define NUM_BUFS 2000

static char *blocks[NUM_BUFS];
static size_t sizes[NUM_BUFS];
static const char *marker = "eNd-Of-bUfFeR";
#define marker_length 13

static size_t lastbuf = 0;

static void
init_xmalloc_debug(void)
{
	for (size_t i = 0; i < NUM_BUFS; ++i) {
		blocks[i] = NULL;
		sizes[i] = 0;
	}
}

static size_t
find_free_slot(void)
{
	for (size_t i = 0; i < lastbuf; ++i) {
		if (blocks[i] == NULL)
			return i;
	}

	++lastbuf;
	if (lastbuf > NUM_BUFS) {
		fprintf(stderr, "xmalloc-debug ran out of buffers!\n");
		exit(1);
	}

	return lastbuf - 1;
}

static size_t
find_slot(void *address, const char *funcname)
{
	for (size_t i = 0; i < lastbuf; ++i) {
		if (blocks[i] == address)
			return i;
	}

	fprintf(stderr, "xmalloc-debug: %s: invalid pointer %p!\n",
	        funcname, address);
	exit(1);
}

void
xmalloc_debug_error(char *buf)
{
	fprintf(stderr, "*** write past allocated buffer end!\n");
	fprintf(stderr, "buf: %s\n", buf);
}

void
xmalloc_debug_check(void)
{
	for (size_t i = 0; i < lastbuf; ++i) {
		if (blocks[i]) {
			if (memcmp(blocks[i] + sizes[i], marker, 
			    marker_length)) {
				xmalloc_debug_error(blocks[i]);
				break;
			}
		}
	}
}

void *
xmalloc_debug(size_t size)
{
	xmalloc_debug_check();
	size_t i = find_free_slot();
	char *retval = malloc(size + marker_length);
	if (!retval) {
		fprintf(stderr, "virtual memory exhausted\n");
		exit(1);
	}
	memcpy(retval + size, marker, marker_length);

	blocks[i] = retval;
	sizes[i] = size;

	return (void *) retval;
}

void *
xrealloc_debug(void *address, size_t size)
{
	if (!address)
		return xmalloc_debug(size);

	xmalloc_debug_check();
	size_t i = find_slot(address, "xrealloc");
	char *retval = realloc(address, size + marker_length);
	if (!retval) {
		fprintf(stderr, "virtual memory exhausted\n");
		exit(1);
	}
	memcpy(retval + size, marker, marker_length);

	blocks[i] = retval;
	sizes[i] = size;

	return (void *) retval;
}

char *
xstrdup_debug(const char *string)
{
	size_t size = 1 + strlen(string);
	char *retval = xmalloc_debug(size);
	strcpy(retval, string);

	return retval;
}

char *
xstrndup_debug(const char *string, size_t n)
{
	size_t size = 1 + n;
	char *retval = xmalloc_debug(size);
	strncpy(retval, string, n);
	retval[n] = '\0';

	return retval;
}

void
xmalloc_debug_free(void *address)
{
	xmalloc_debug_check();
	size_t i = find_slot(address, "free");
	blocks[i] = NULL;
	sizes[i] = 0;

#if defined(free_orig)
	free_orig(address);
#else
	free(address);
#endif
}
