import { FileCpu } from "@wxn0brp/db-core/types/fileCpu";
import { VQueryT } from "@wxn0brp/db-core/types/query";
import { findUtil } from "@wxn0brp/db-core/utils/action";
import { vFileCpu } from "@wxn0brp/db-storage-dir";
import { FileActions } from "@wxn0brp/db-storage-dir/action";
import { DbDirOpts } from "@wxn0brp/db-storage-dir/types";
import { find } from "./native";

export class NativeActions extends FileActions {
    constructor(
        folder: string,
        options: DbDirOpts,
        fileCpu: FileCpu,
    ) {
        super(folder, options, fileCpu);
    }

    async find(query: VQueryT.Find) {
        await this.ensureCollection(query.collection);
        this._ensureQueryFormat(query);

        const c_path = this._getCollectionPath(query.collection);

        const data = find(c_path, query.search, false);
        // TODO update when new core release
        const res = await findUtil(query, {
            find() {
                return data;
            }
        } as any, [""]);
        return res || [];
    }

    async findOne(query: VQueryT.FindOne) {
        await this.ensureCollection(query.collection);
        this._ensureQueryFormat(query);

        const c_path = this._getCollectionPath(query.collection);
        const data = find(c_path, query.search, true);
        return data || null;
    }
}

export const DYNAMIC = {
    "dir-c": (folder: string, options: DbDirOpts = {}) => new NativeActions(folder, options, vFileCpu)
}
