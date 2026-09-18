#ifndef Env_h
#define Env_h
#include "../Self.h"

#include "../../../Interface/Core/String.h"

err_t environment_variable_get   (charseq_t key, string_t* out_string);
err_t environment_variable_set   (charseq_t key, charseq_t value);
err_t environment_variable_unset (charseq_t key);

#define env_get(k, out) environment_variable_get((k), (out))
#define env_set(k, v)   environment_variable_set((k), (v))
#define env_unset(k)    environment_variable_unset((k))

err_t environment_variables_load   (charseq_t filename, uint64_t* out_loaded_count);
err_t environment_variables_unload (charseq_t filename); // Not Implemented yet!!!

#define env_load(f, out) (environment_variables_load((f), (out)))
#define env_unload(f)    (environment_variables_unload((f)))

#endif // Env_h