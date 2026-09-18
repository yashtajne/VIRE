#ifndef Path_h
#define Path_h
#include "../Self.h"

#include "../../Core/String.h"

typedef string_t  path_t;
typedef charseq_t pathname_t;

err_t path_getCurrentDirectory(pathname_t* pathname);

boolean path_isAbsolutePath (path_t* path, err_t* occured);
boolean path_isRelativePath (path_t* path, err_t* occured);

boolean path_isDirectoryPath (path_t* path, err_t* occured);
boolean path_isFilePath      (path_t* path, err_t* occured);

boolean path_existsPath (path_t* path, err_t* occured);

boolean pathname_isAbsolute (pathname_t pathname, err_t* occured);
boolean pathname_isRelative (pathname_t pathname, err_t* occured);

boolean pathname_isDirectory (pathname_t pathname, err_t* occured);
boolean pathname_isFile      (pathname_t pathname, err_t* occured);

boolean pathname_exists (pathname_t pathname, err_t* occured);

#define path_argument(path) \
	_Generic((path), \
		path_t: &(path), \
		path_t*: (path), \
		pathname_t: (path))

#define path_isAbsolute(path, occured) \
	_Generic((path), \
		path_t: path_isAbsolutePath, \
		path_t*: path_isAbsolutePath, \
		pathname_t: pathname_isAbsolute \
	)(path_argument(path), (occured))

#define path_isRelative(path, occured) \
	_Generic((path), \
		path_t: path_isRelativePath, \
		path_t*: path_isRelativePath, \
		pathname_t: pathname_isRelative \
	)(path_argument(path), (occured))

#define path_isDirectory(path, occured) \
	_Generic((path), \
		path_t: path_isDirectoryPath, \
		path_t*: path_isDirectoryPath, \
		pathname_t: pathname_isDirectory \
	)(path_argument(path), (occured))

#define path_isFile(path, occured) \
	_Generic((path), \
		path_t: path_isFilePath, \
		path_t*: path_isFilePath, \
		pathname_t: pathname_isFile \
	)(path_argument(path), (occured))

#define path_exists(path, occured) \
	_Generic((path), \
		path_t: path_existsPath, \
		path_t*: path_existsPath, \
		pathname_t: pathname_exists \
	)(path_argument(path), (occured))

#endif // Path_h
