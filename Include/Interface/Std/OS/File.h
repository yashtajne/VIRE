#ifndef File_h
#define File_h
#include "../Self.h"

#include "../../Core/Error.h"
#include "Path.h"

struct _File
{
#if IS_PLATFORM_WINDOWS
	void* handle;
	uint64_t cursor;
	uint64_t size;
#elif IS_PLATFORM_LINUX
	int32_t descriptor;
	uint64_t cursor;
	uint64_t size;
#endif
};
typedef struct _File file_t;

/*
|--------|--------------------------|---------------------------|------------------------|
| Mode   | Operation                | If file exists            | If file does not exist |
|--------|--------------------------|---------------------------|------------------------|
| "r"    | Read Only                | OK                        | ERROR_FILE_NOT_FOUND   |
| "w"    | Write Only               | OK (truncates)            | OK (creates new file)  |
| "a"    | Write Only (appends)     | OK (keeps data)           | OK (creates new file)  |
| "r+w"  | Read and Write           | OK (keeps data)           | ERROR_FILE_NOT_FOUND   |
| "r+a"  | Read and Write (appends) | OK (keeps data)           | OK (creates new file)  |
| "wx"   | Write Only               | ERROR_FILE_ALREADY_EXISTS | OK (creates new file)  |
| "r+wx" | Write Only               | ERROR_FILE_ALREADY_EXISTS | OK (creates new file)  |
|--------|--------------------------|---------------------------|------------------------|
*/

file_t* file_open  (charseq_t filename, charseq_t mode, err_t* occured);
err_t   file_close (file_t* file);

#define file_delete(P) \
	_Generic((P), \
		path_t*: file_deleteUsingPath, \
		pathname_t: file_deleteUsingPathname \
	)(P)

err_t file_deleteUsingPath     (path_t* file);
err_t file_deleteUsingPathname (pathname_t pathname);

char_t file_getCharacter (file_t* file, err_t* occured);
err_t  file_putCharacter (file_t* file, char_t character);

uint64_t file_getCursorPosition (file_t* file, err_t* occured);
err_t    file_setCursorPosition (file_t* file, uint64_t position);
err_t    file_moveCursor        (file_t* file, int64_t offset);

err_t file_readBytes   (file_t* file, byte_t* bytes, uint64_t number_of_bytes_to_read,  uint64_t* number_of_bytes_read);
err_t file_writeBytes (file_t* file, byte_t* bytes, uint64_t number_of_bytes_to_write, uint64_t* number_of_bytes_written);

/*
err_t file_readLine  (file_t* file, string_t* out_line);
err_t file_writeLine (file_t* file, string_t* line, uint64_t* out_bytes_written);

err_t get_file_size  (file_t* file, uint64_t* out_file_size);
*/

#endif // File_h
