#ifndef CoreChar_h
#define CoreChar_h
#include "../../Pure.h"

static inline boolean is_space        (char);
static inline boolean is_digit        (char);
static inline boolean is_alphabetic   (char);
static inline boolean is_alphanumeric (char);

static inline uint64_t charseq_countbytes (char* charseq);

boolean charseq_hasPrefix (charseq_t original, charseq_t prefix, err_t* occured);
boolean charseq_hasSuffix (charseq_t original, charseq_t suffix, err_t* occured);

boolean charseq_equals (charseq_t original, charseq_t compare_with, err_t* occured);

err_t charseq_toUppercase (charseq_t);
err_t charseq_toLowercase (charseq_t);

boolean charseq_contains (charseq_t, charseq_t subseq, err_t* occured);
boolean charseq_find     (charseq_t, charseq_t subseq, uint64_t* position, err_t* occured);

// static inline void charseq_replace (charseq_t this, charseq_t target, charseq_t replacement);

static inline boolean
is_space(char character)
{
	return character == ' ' || (unsigned)character - '\t' < 5;
}

static inline boolean
is_digit(char character)
{
	return character >= '0' && character <= '9';
}

static inline boolean
is_alphabetic(char character)
{
	return ( (unsigned)character | 32 ) - 'a' < 26;
}

static inline boolean
is_alphanumeric(char character)
{
	return is_alphabetic(character) || is_digit(character);
}

static inline uint64_t
charseq_countbytes(const charseq_t charseq)
{
	if ( !charseq ) return 0;

	uint64_t count = 0;
	while ( charseq[count] != '\0' )
		count++;

	return count;
}

#endif // Char_h
