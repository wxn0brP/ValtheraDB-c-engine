import { CString, dlopen, FFIType } from "bun:ffi";

const lib = dlopen("./lib/libvaltheradb.so", {
    find: {
        args: [FFIType.cstring, FFIType.cstring, FFIType.bool],
        returns: FFIType.ptr
    },
    free_result: {
        args: [FFIType.ptr],
        returns: FFIType.void
    }
});

function encode(str: string) {
    return new TextEncoder().encode(str + "\0");
}

export function find(dir: string, fields: object, findOne = false) {
    const json = JSON.stringify(fields);

    const ptr = lib.symbols.find(encode(dir), encode(json), findOne);
    if (!ptr)
        throw new Error("Error. Pointer is null");
    const result = new CString(ptr).toString();

    lib.symbols.free_result(ptr);
    return JSON.parse(result);
}
