#pragma once

#include <libc.h>
#include <stdint.h>
#include <types.h>


/* This macro required "errors" variable to be available */
#define TEST_EQUAL(x,ex) if((x) != (ex)) {\
				errors += 1;\
				DEBUGF("Test failed. 0x%x != 0x%x \n",\
				       (uint64_t)x, (uint64_t)ex); }

#define TEST_NOT_EQUAL(x,ex) if((x) == (ex)) {\
				errors += 1;\
				DEBUGF("Test failed. 0x%x == 0x%x \n",\
				       (uint64_t)x, (uint64_t)ex); }


uint64_t tree_test(void);
uint64_t list_test(void);

/* Returns the number of errors */
uint64_t test_all(void);
