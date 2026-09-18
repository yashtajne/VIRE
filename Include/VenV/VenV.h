#ifndef VenV_h
#define VenV_h

/*
 * VenV - Virtual Environment Engine
 * Main Header File
 * 
 * A lightweight virtual environment engine that emulates
 * complete computer environments with custom CPU architecture.
 */

#include "../Core/Int.h"
#include "../Core/Error.h"

/*---- Version ----*/

#define VENV_VERSION_MAJOR    0
#define VENV_VERSION_MINOR    1
#define VENV_VERSION_PATCH    0
#define VENV_VERSION_STRING   "0.1.0"

/*---- Include All Components ----*/

#include "ISA.h"
#include "CPU.h"
#include "Memory.h"
#include "Device.h"
#include "Timer.h"
#include "Console.h"
#include "Block.h"
#include "Net.h"
#include "VM.h"
#include "Assembler.h"

/*---- Engine Initialization ----*/

/* Initialize the VenV engine (call once at startup) */
err_t venv_init(void);

/* Shutdown the VenV engine (call at exit) */
void venv_shutdown(void);

/* Get version string */
const char* venv_version(void);

/*---- Utility Functions ----*/

/* Convert error code to string */
const char* venv_strerror(err_t err);

/* Enable/disable logging */
void venv_set_logging(boolean enabled);

#endif /* VenV_h */
