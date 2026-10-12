# Command-line Interface

Ripes v.2.2.5 adds support for a command line interface. Through this, programs can be assembled/compiled and simulated on any of the available processor models.

An example execution could be:
```sh
./Ripes 
  --mode cli            \ # Enable command line interface
  --src foo.s           \ # Input file
  -t asm                \ # Input file type
  --proc "RV32_5S"      \ # Processor model
  --isaexts M,C         \ # ISA extensions to enable in the assembler/processor  
  --ipc                 \ # show IPC
  --cycles              \ # show # of cycles executed
  --pipeline              # Show pipeline state during execution
```

## Options

See `./Ripes --help` for further information.

| *Flag* | *Description* |
| ---- | ----------- |
|  --mode <mode>       |  Ripes mode Options: `(gui, cli)` |
|  --src <src>         |  Source file |
|  -t <type>           |  Source type. Options: `(c, asm, bin)` |
|  --proc <proc>       |  Processor model (see `./Ripes --help` for options). |
|  --isaexts <isaexts> |  ISA extensions to enable (comma separated). |
|  --timeout <timeout> |  Simulation timeout in milliseconds. If simulation does not finish within the specified time, it will be aborted. |
|  -v                  |  Verbose output and runtime status information. |
|  --output <output>   |  Report output file. If not set, report is printed to stdout. |
|  --json              |  JSON-formatted report. |
|  --all               |  Enable all report options. |
|  --cycles            |  Report cycles |
|  --iret              |  Report instructions retired |
|  --instruction_histogram | Report retired instruction counts, with expanded compressed mnemonics and one-based source locations |
|  --cpi               |  Report cycles per instruction (CPI) |
|  --ipc               |  Report instructions per cycle (IPC) |
|  --pipeline          |  Report pipeline state |
|  --regs              |  Report register values |
|  --runinfo           |  Report simulation information in output (processor configuration, input file, ...) |
|   --reginit <[rid:v]>|     Comma-separated list of register initialization values. The register value may be specified in signed, hex, or boolean notation. Format: `<register idx>=<value>,<register idx>=<value>` |

## Retired instruction telemetry

`--instruction_histogram` (also enabled by `--all`) records actual machine
encodings and PCs at retirement. All RISC-V processor models support it. Pipeline
models carry the fetched encoding through their valid/flush-controlled registers;
the report does not read assembly source or re-read memory at the retiring PC.
Stalled cycles and flushed instructions do not add counts. Both dual-issue lanes
are observed. Compressed instructions are expanded, so `c.slli` counts as `slli`.

With `--json`, `instruction_histogram` has these fields:

- `supported`: whether the processor supplies retirement events.
- `counts`: mnemonic to count for the whole run, including any caller scaffolding.
- `by_line`: one-based assembler source line to mnemonic counts. Consumers can
  exclude their own declared prologue lines without inspecting submitted source.
- `unmapped`: counts for addresses without assembler source locations, such as
  an externally loaded binary. Raw encodings emitted in `.text` by directives
  retain source locations. An undecodable retired word is counted as `unknown`.

Counts are 64-bit unsigned values and may serialize as strings. The histogram
describes the clocks completed in that run, including a run stopped at a cycle
bound; it is reset for each run. Retirement demonstrates that an instruction
executed, not that it contributed to the program's result.


