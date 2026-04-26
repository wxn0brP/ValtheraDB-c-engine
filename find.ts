import { CString, dlopen, FFIType } from "bun:ffi";

const lib = dlopen("./libfind.so", {
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

function find(file: string, fields: object, findOne = false) {
    const json = JSON.stringify(fields);

    const ptr = lib.symbols.find(encode(file), encode(json), findOne);
    const result = new CString(ptr).toString();

    lib.symbols.free_result(ptr);
    return result;
}

console.log(find("./test.jsonl", { id: 1_999_999 }, true));
