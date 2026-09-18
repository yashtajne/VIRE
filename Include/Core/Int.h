#ifndef CoreInt_h
#define CoreInt_h
#include "../../Pure.h"

static inline uint64_t integer_countdigits (int64_t integer);

static inline uint64_t
integer_countdigits(int64_t num)
{
	int count = 0;

	if (num == 0)
		return 1;

	if ( num < 0 )
	{
		num = -num;
	}

	while ( num > 0 )
	{
		count++;
		num /= 10;
	}

	return count;
}

#endif // Int_h