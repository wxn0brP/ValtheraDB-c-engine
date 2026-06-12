import { CString, dlopen, FFIType } from "bun:ffi";
import { join } from "path";

const path = join(import.meta.dir, "..", "lib", "libvaltheradb.so");
const lib = dlopen(path, {
    find: {
        args: [FFIType.cstring, FFIType.cstring, FFIType.bool],
        returns: FFIType.ptr
    },
    free_result: {
        args: [FFIType.ptr],
        returns: FFIType.void
    }
});

const encoder = new TextEncoder();

function encode(str: string) {
    return encoder.encode(str + "\0");
}

export function find(dir: string, fields: object, findOne = false) {
    const json = JSON.stringify(fields);

    const ptr = lib.symbols.find(encode(dir), encode(json), findOne);
    if (!ptr)
        throw new Error("Error. Pointer is null");

    let result: string;

    try {
        result = new CString(ptr).toString();
    } finally {
        lib.symbols.free_result(ptr);
    }

    return JSON.parse(result);
}
