# VIRE - Virtual Isolated Runtime Environment

A lightweight virtual environment engine that emulates complete computer environments with a custom CPU architecture.

## Overview

VIRE (Virtual Isolated Runtime Environment) is a **virtual hardware emulator**, not an OS-level container like Docker. Each environment contains:

- A custom virtual CPU (VIRE ISA - not based on x86, ARM, RISC-V, or MIPS)
- Virtual RAM with sparse allocation
- Virtual block device (disk)
- Virtual console
- Virtual timer
- Virtual network interface

The engine provides **virtual hardware**. The guest operating system (eventually Linux) provides all OS functionality.

## Architecture

```
HOST
│
└── VIRE Engine
     │
     ├── Virtual CPU (custom ISA)
     ├── Virtual RAM
     ├── Virtual Disk → disk.img
     ├── Virtual Console → host stdout/stdin
     ├── Virtual Timer → interrupts
     └── Virtual NIC → host network
             │
             ▼
          Guest OS (Linux)
               │
               ├── Processes
               ├── Filesystem (ext4, etc.)
               ├── TCP/IP stack
               └── Applications
```

## Project Structure

```
/workspace/
├── Include/
│   ├── Core/           # Core runtime (freestanding C utilities)
│   ├── Std/            # Standard library interface layer
│   └── VenV/           # VIRE engine headers
│       ├── VenV.h      # Main header
│       ├── ISA.h       # CPU instruction set architecture
│       ├── CPU.h       # CPU state and operations
│       ├── Memory.h    # Virtual memory subsystem
│       ├── Device.h    # Virtual device interface
│       ├── Timer.h     # Virtual timer device
│       ├── Console.h   # Virtual console device
│       ├── Block.h     # Virtual block device
│       ├── Net.h       # Virtual network device
│       ├── VM.h        # Virtual machine abstraction
│       └── Assembler.h # Assembler for VIRE ISA
├── Src/
│   ├── CPU.c           # CPU implementation
│   ├── Memory.c        # Memory implementation
│   └── Assembler.c     # Assembler implementation
├── Test/
│   └── test_vire.c     # Test program
├── Environments/       # VM instances (created at runtime)
├── Pure.h              # Freestanding runtime definitions
└── Readme.md
```

## Custom CPU ISA

The VIRE ISA is a completely new 64-bit architecture designed for efficient emulation:

### Registers
- 16 general-purpose 64-bit registers (x0-x15)
- x0 is hardwired to zero
- x14 is stack pointer (sp)
- x15 is return address (ra)
- Program counter (pc)
- Status register (interrupts, privilege level)

### Instruction Format
- Fixed 32-bit instructions
- Little-endian encoding
- Simple opcode/register/immediate fields

### Instructions
- **Load/Store**: lb, lh, lw, ld, sb, sh, sw, sd
- **Arithmetic**: add, sub, mul, div, rem
- **Logical**: and, or, xor
- **Shift**: sll, srl, sra
- **Compare**: slt, sltu
- **Branch**: beq, bne, blt, bge, bltu, bgeu
- **Jump**: jal, jalr
- **Immediate**: addi, andi, ori, xori, slli, srli, srai, slti, sltiu
- **System**: ecall, ebreak, mret, fence, wfi

### Privilege Levels
- User mode (least privileged)
- Supervisor mode
- Machine mode (most privileged)

### Exceptions & Interrupts
- Instruction faults
- Load/store faults
- System calls (ecall)
- Breakpoints (ebreak)
- Timer interrupts
- External interrupts

## Development Phases

### Phase 1: Core CPU (Current)
- [x] ISA definition
- [x] CPU interpreter
- [x] Virtual memory
- [x] Basic assembler
- [ ] Test programs

### Phase 2: Devices
- [ ] Timer device
- [ ] Console device
- [ ] Block device
- [ ] MMIO framework

### Phase 3: End-to-End
- [ ] Run simple programs
- [ ] Console I/O
- [ ] Disk I/O

### Phase 4: Networking
- [ ] Virtual NIC
- [ ] Host network bridge
- [ ] Port forwarding

### Phase 5: OS Support
- [ ] Full interrupt controller
- [ ] Memory management unit
- [ ] Boot process

### Phase 6: Linux Port
- [ ] Linux kernel port
- [ ] Device drivers
- [ ] Minimal root filesystem

## Building

**Important:** This project uses a freestanding C runtime without libc dependency. All standard library functions are provided by the Interface layer in `Include/Interface/`.

### Prerequisites
- Clang or GCC with C99 support
- No external dependencies required

### Compile Commands

#### Windows (using Clang)
```batch
:: Basic compilation with relative includes
clang -nostdlib -ffreestanding -o test_vire.exe Test/test_vire.c Src/CPU.c Src/Memory.c Src/Assembler.c
```

#### Linux/macOS
```bash
# Basic compilation with relative includes
clang -nostdlib -ffreestanding -o test_vire Test/test_vire.c Src/CPU.c Src/Memory.c Src/Assembler.c
```

### Notes
- **No `-I` include flags**: All includes use relative paths from source files
- **No libc**: Uses `-nostdlib -ffreestanding` flags
- **Custom runtime**: String/memory functions provided in `Include/Core/String.h`
- **Boolean types**: Use `TRUE`/`FALSE` from `Pure.h` or `pure_true`/`pure_false`

## Usage (Future CLI)

```bash
# Create a new environment
vire create myenv --image minimal-linux

# Start environment
vire start myenv

# Execute command in environment
vire exec myenv /bin/sh

# Port forwarding
vire start web --port 8080:80

# Stop environment
vire stop myenv

# Destroy environment
vire destroy myenv
```

## Design Principles

1. **Emulate hardware, not OS** - The engine provides virtual devices; Linux provides filesystems, networking, processes.

2. **Simple and efficient** - The ISA is designed for fast interpretation, with potential for future JIT compilation.

3. **Minimal footprint** - Sparse memory allocation, lazy device initialization.

4. **Clear boundaries** - Well-defined interfaces between CPU, memory, and devices.

5. **Extensible** - Designed to eventually support full Linux virtualization.

6. **No libc dependency** - All standard library functions are implemented in the Interface layer for complete control and portability.

## Quick Start

```bash
# Clone the repository
git clone https://github.com/yashtajne/VIRE.git
cd VIRE

# Build the test program
clang -nostdlib -ffreestanding -o test_vire Test/test_vire.c Src/CPU.c Src/Memory.c Src/Assembler.c

# Run tests
./test_vire
```

## License

See LICENSE file.
