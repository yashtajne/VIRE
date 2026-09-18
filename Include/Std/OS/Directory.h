#ifndef Directory_h
#define Directory_h
#include "../Self.h"

#include "../../Core/Error.h"

err_t create_directory (charseq_t pathname);
err_t delete_directory (charseq_t pathname);

/*
struct DirectoryWalker
{
#if IS_PLATFORM_WINDOWS
	HANDLE _windows_directory_walker_file_handle;
	WIN32_FIND_DATAA _windows_directory_walker_data;
#else
	#error "Not yet Implemented!"
#endif
};

err_t directory_walk_start (struct DirectoryWalker* walker, charseq_t wildcard);
err_t directory_walk_next  (struct DirectoryWalker* walker);
err_t directory_walk_stop  (struct DirectoryWalker* walker);

static inline charseq_t
directory_walker_get_filename(struct DirectoryWalker* walker)
{
	if ( !walker ) return NULL;
#if IS_PLATFORM_WINDOWS
	return walker->_windows_directory_walker_data.cFileName;
#else
	#error "Not yet Implemented!"
#endif
}

static inline uint64_t
directory_walker_get_filesize(struct DirectoryWalker* walker)
{
	if ( !walker ) return 0;
#if IS_PLATFORM_WINDOWS
	return ((uint64_t)walker->_windows_directory_walker_data.nFileSizeHigh << 32) | (uint32_t)walker->_windows_directory_walker_data.nFileSizeLow;
#else
	#error "Not yet Implemented!"
#endif
}
*/
#endif // Directory_H