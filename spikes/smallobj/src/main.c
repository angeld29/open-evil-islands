#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "smallobj.h"

int main()
{
	srand(time(NULL));

	if (!smallobj_open()) {
		printf("Could not open smallobj\n");
	}

	const size_t count = 25000;
	const size_t size = 8;

	void* p[count];

	for (size_t j = 0; j < 100; ++j) {
		for (size_t i = 0; i < count; ++i) {
			p[i] = smallobj_alloc(size);

			if (NULL == p[i]) {
				printf("smallobj alloc failed\n");
			}
		}

		for (size_t i = 0; i < 1000; ++i) {
			int idx1 = rand() % count;
			int idx2 = rand() % count;
			void* t = p[idx1];
			p[idx1] = p[idx2];
			p[idx2] = t;
		}

		for (size_t i = 0; i < count; ++i) {
			smallobj_free(p[i], size);
		}
	}

	smallobj_close();
	return 0;
}
