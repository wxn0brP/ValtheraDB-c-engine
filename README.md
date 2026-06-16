# ValtheraDB C Engine

Adapter providing C backend for ValtheraDB.

## Supported Operations

Currently only: 

- `find`
- `findOne`
- `remove`
- `removeOne`
- `update`
- `updateOne`
- `updateOneOrAdd`

are partially implemented in native C. Other operations fall back to TS implementation.

## Installation

Build the native library first:

```bash
./build.sh
```

Then use with `@wxn0brp/db-core`.

## Usage

```typescript
import { NativeActions } from "@wxn0brp/db-c";
import { ValtheraClass } from "@wxn0brp/db-core";
import { vFileCpu } from "@wxn0brp/db-storage-dir";

const adapter = new NativeActions(
    "./data",
    {},
    vFileCpu
)

const db = new ValtheraClass({ dbAction: adapter });
```

## Requirements

- Linux x64/arm64
- Bun runtime

## License

MIT
