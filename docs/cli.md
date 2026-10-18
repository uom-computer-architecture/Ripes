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
|  --require-any-instruction <mnemonics> | Require one of the comma-separated canonical machine instructions to retire |
|  --instruction-source-start <line> | First eligible assembly source line, numbered from 1 and inclusive (default 1); requires an instruction requirement |
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
  retain source locations. CLI execution stops before an undecodable candidate
  retires; that fault does not add a retired count.

Counts are 64-bit unsigned values and may serialize as strings. The histogram
describes the clocks completed in that run, including a run stopped at a cycle
bound; it is reset for each run. Retirement demonstrates that an instruction
executed, not that it contributed to the program's result.

## Executed instruction requirements

`--require-any-instruction sll,slli` asks Ripes to evaluate the requirement from
its captured retirement evidence. It enables collection even without `--all` or
`--instruction_histogram`. Use real instructions from the selected ISA, not
pseudo-instructions such as `li`. Compressed execution uses expanded names:
request `slli`, not `c.slli`, to accept either encoding.

`--instruction-source-start N` excludes caller scaffolding before source line
`N`; the boundary is inclusive. With the default of 1, the entire run qualifies,
including externally loaded binaries. A boundary greater than 1 requires assembly
source and uses the assembler address mapping. Missing locations or mappings
that cross the boundary produce an error, never a failed student requirement.
No source regex or memory re-read is involved.

JSON results include `instruction_requirement` when a requirement was requested:

```json
{
  "status": "met",
  "requested": ["sll", "slli"],
  "counts": {"sll": "0", "slli": "1"}
}
```

The verdict is `met`, `not_met`, or `error`; counts include every requested
mnemonic and only qualifying retired instructions. An error includes `reason`:
`retirement_unsupported`, `source_unmapped`, `source_ambiguous`, or
`unknown_retired_instruction`. Invalid requirements or source boundaries return
top-level `status: input_error`, `kind: invalid_instruction_requirement`, and a
`message`, with exit code 1. A `not_met` verdict leaves the simulation status and
exit code successful: consumers decide how to use that observable. Existing
assembly, syscall and termination errors retain their precedence.

## Runtime instruction faults and source diagnostics

CLI execution checks captured instructions with the selected ISA's matcher,
including its enabled extensions, before retirement. Invalid candidates stop
the run; valid raw `.word` instructions, unreachable data and flushed pipeline
instructions remain permitted. The check uses the captured encoding, never a
later memory read at its PC. Compressed decoding preserves unrecognised words
so they cannot silently expand into valid operations.

A fault returns exit code 1 and JSON `status: runtime_error`. It always includes
`exit`, even when no optional telemetry was requested:

```json
{"status":"runtime_error","exit":{"reason":"illegal_instruction","cycles":0,
 "instruction":{"pc":"0","encoding":"0","line":1,"source_lines":[1]}}}
```

`encoding` is the captured (normally expanded) word. `pc` and `encoding` may
serialize as unsigned integer strings. Source lines are one-based; `line` is
present only for an unambiguous mapping, and binary/dynamic code can lack source
locations. Consumers should retain the PC and must not invent a source line.
A processor without captured instruction observations returns
`status: infrastructure_error`, `kind: runtime_observation_unsupported`.

Assembler diagnostics preserve the original mnemonic and operand provenance
through pseudo expansion and identify the base inside a memory operand, rather
than exposing the encoder's field indices. Invalid register operands report
the expected type and offending token, without proposing replacement code.
Semicolon separators are unsupported by this Ripes assembler; that restriction
does not describe every RISC-V assembler. Strings and character literals retain
their semicolons.

The existing wall-clock `status: timeout` reports `timeout_ms`. It establishes
only that elapsed time expired, not an infinite loop or missing exit. It is
distinct from `exit.reason: max_cycles`, the deterministic simulation bound.


