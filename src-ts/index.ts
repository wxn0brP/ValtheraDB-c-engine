import { FileCpu } from "@wxn0brp/db-core/types/fileCpu";
import { VQueryT } from "@wxn0brp/db-core/types/query";
import { findUtil } from "@wxn0brp/db-core/utils/action";
import { vFileCpu } from "@wxn0brp/db-storage-dir";
import { FileActions } from "@wxn0brp/db-storage-dir/action";
import { DbDirOpts } from "@wxn0brp/db-storage-dir/types";
import { find, findPaged, remove } from "./native";

function isPlainSearch(search: unknown) {
    if (search === undefined || search === null) return true;
    if (typeof search !== "object" || Array.isArray(search)) return false;

    const proto = Object.getPrototypeOf(search);
    return proto === Object.prototype || proto === null;
}

function hasOptions(value: unknown) {
    return !!value && typeof value === "object" && Object.keys(value).length > 0;
}

function canPageNatively(dbFindOpts: VQueryT.Find["dbFindOpts"]) {
    if (!dbFindOpts) return false;

    const {
        reverse,
        sortBy,
        min,
        max,
        avg,
        groupBy,
        count,
    } = dbFindOpts;

    return !reverse && !sortBy && !min && !max && !avg && !groupBy && !count;
}

function hasPaging(dbFindOpts: VQueryT.Find["dbFindOpts"]) {
    if (!dbFindOpts) return false;
    return dbFindOpts.offset !== undefined || dbFindOpts.limit !== undefined;
}

export class NativeActions extends FileActions {
    constructor(
        folder: string,
        options: DbDirOpts,
        fileCpu: FileCpu,
    ) {
        super(folder, options, fileCpu);
    }

    private _canUseNative(query: VQueryT.Find | VQueryT.FindOne | VQueryT.Remove) {
        const format = this.options.format;
        if (format !== "json") return false;
        if (this.options.delimiter && this.options.delimiter !== "\n") return false;
        if (this.options.stringifyArgs?.length) return false;
        return isPlainSearch(query.search);
    }

    async find(query: VQueryT.Find) {
        if (!this._canUseNative(query) || hasOptions(query.findOpts))
            return super.find(query);

        await this.ensureCollection(query.collection);
        this._ensureQueryFormat(query);

        const c_path = this._getCollectionPath(query.collection);

        if (hasPaging(query.dbFindOpts) && canPageNatively(query.dbFindOpts)) {
            const { offset = 0, limit = -1 } = query.dbFindOpts!;
            return findPaged(c_path, query.search, offset, limit);
        }

        const data = find(c_path, query.search, false);
        const res = await findUtil(query, data, [""]);
        return res || [];
    }

    async findOne(query: VQueryT.FindOne) {
        if (!this._canUseNative(query) || hasOptions(query.findOpts))
            return super.findOne(query);

        await this.ensureCollection(query.collection);
        this._ensureQueryFormat(query);

        const c_path = this._getCollectionPath(query.collection);
        const data = find(c_path, query.search, true);
        return data || null;
    }

    async remove(query: VQueryT.Remove) {
        if (!this._canUseNative(query))
            return super.remove(query);

        await this.ensureCollection(query.collection);
        this._ensureQueryFormat(query);

        const c_path = this._getCollectionPath(query.collection);
        return remove(c_path, query.search, false);
    }

    async removeOne(query: VQueryT.Remove) {
        if (!this._canUseNative(query))
            return super.removeOne(query);

        await this.ensureCollection(query.collection);
        this._ensureQueryFormat(query);

        const c_path = this._getCollectionPath(query.collection);
        const data = remove(c_path, query.search, true);
        return data[0] ?? null;
    }
}

export const DYNAMIC = {
    "dir-c": (folder: string, options: DbDirOpts = {}) => new NativeActions(folder, options, vFileCpu)
}
