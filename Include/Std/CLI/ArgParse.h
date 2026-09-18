/* Date: 8/12/26
Argument Parser
Only Supports parsing subcommands for now
*/
#ifndef StdCLIArgParse_h
#define StdCLIArgParse_h

#include "../Array.h"
#include "../String.h"

struct ParsedArguments
{
	string_t ProcessName;
	array_t* SubCommands;
};

struct ArgFlag
{
	char_t     character;
	boolean    is_value;
	union {
		boolean*   toggelable;
		charseq_t* value;
	};

	charseq_t  description_short;
	charseq_t  description_full;
};
typedef struct ArgFlag ArgFlag;

#define bool_flag(C, P, DS, DP)               \
	(struct ArgFlag){                         \
		.character         = (C),             \
		.is_value          = false,           \
		.toggelable        = (P),             \
		.description_short = (charseq_t)(DS), \
		.description_full  = (charseq_t)(DP)  \
	}

#define value_flag(C, V, DS, DP)              \
	(struct ArgFlag){                         \
		.character         = (C),             \
		.is_value          = true,            \
		.value             = (V),             \
		.description_short = (charseq_t)(DS), \
		.description_full  = (charseq_t)(DP)  \
	}

#define flag(C, P, DS, DP) bool_flag(C, P, DS, DP)
#define bflag(C, P, DS, DP) bool_flag(C, P, DS, DP)
#define vflag(C, V, DS, DP) value_flag(C, V, DS, DP)

struct ParsedArguments* arguments_parse (uint64_t argc, char** argv, err_t* occured, struct ArgFlag flags_schema[]);
err_t                   arguments_free  (struct ParsedArguments*);

#endif // StdCLIArgParse_h