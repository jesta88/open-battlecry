#include <stdio.h>
#include "xcr.h"

static wb_xcr* xcr;

int main(int argc, char *argv[])
{
	int result = 0;
	result = wb_xcr_load("Barbarians.xcr", &xcr);

	printf("Result: %d\n", result);

	wb_xcr_unload(xcr);

    return result;
}