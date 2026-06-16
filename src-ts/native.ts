import { CString, dlopen, FFIType } from "bun:ffi";
import { join } from "path";

const path = join(import.meta.dir, "..", "lib", "libvaltheradb.so");
const lib = dlopen(path, {
    find: {
        args: [FFIType.cstring, FFIType.cstring, FFIType.bool],
        returns: FFIType.ptr
    },
    find_paged: {
        args: [FFIType.cstring, FFIType.cstring, FFIType.int32_t, FFIType.int32_t],
        returns: FFIType.ptr
    },
    remove_entries: {
        args: [FFIType.cstring, FFIType.cstring, FFIType.bool],
        returns: FFIType.ptr
    },
    update_entries: {
        args: [FFIType.cstring, FFIType.cstring, FFIType.cstring, FFIType.bool],
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

function parseResult(ptr: any) {
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

function callJson(symbol: "find" | "remove_entries", dir: string, fields: unknown, flag: boolean) {
    const json = JSON.stringify(fields ?? {});
    const ptr = lib.symbols[symbol](encode(dir), encode(json), flag);
    return parseResult(ptr);
}

export function find(dir: string, fields: unknown, findOne = false) {
    return callJson("find", dir, fields, findOne);
}

export function findPaged(dir: string, fields: unknown, offset: number, limit: number) {
    const json = JSON.stringify(fields ?? {});
    const ptr = lib.symbols.find_paged(encode(dir), encode(json), offset, limit);
    return parseResult(ptr);
}

export function remove(dir: string, fields: unknown, one = false) {
    return callJson("remove_entries", dir, fields, one);
}

export function update(dir: string, fields: unknown, updater: unknown, one = false) {
    const fieldsJson = JSON.stringify(fields ?? {});
    const updaterJson = JSON.stringify(updater ?? {});
    const ptr = lib.symbols.update_entries(encode(dir), encode(fieldsJson), encode(updaterJson), one);
    return parseResult(ptr);
}
