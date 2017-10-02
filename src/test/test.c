#include "test.h"

uint64_t test_all(void)
{
	uint64_t errors = 0x00;

	errors += list_test();
	errors += tree_test();
	DEBUGF("Tests failed: 0x%x\n", errors);

	return errors;
}
