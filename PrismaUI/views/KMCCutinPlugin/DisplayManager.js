import { DisplayDrawingTexture } from "./DisplayDrawingTexture.js";
import { DisplayStopIconTexture } from "./DisplayStopIconTexture.js";
import { DisplayProfile } from "./DisplayProfile.js";

// カットイン関連の大本
// ID = (0)=>プレイヤー (1以降)=>フォロワー
// group = category_id なんでこんな名前にしたんだろ面倒なので今度直す
// TODO:カットインドラッグできるけどドラッグ後に位置保存してないので意味ない。外だし。

// stop iconからstop allとか呼ぶ
export class DisplayManager {
    constructor() {
        this.displayMap = new Map();
        this.stopIcon = null;
        this.profile = new DisplayProfile();
    }

    initStopIcon() {
        if (!this.stopIcon) {
            const canvas = document.getElementById("common_stop_icon");
            if (canvas) {
                this.stopIcon = new DisplayStopIconTexture(canvas);
            }
        }
    }

    create(id, canvas) {
        const display = new DisplayDrawingTexture(canvas);
        this.displayMap.set(id, display);
    }

    async play(id, group, next_group, name) {
        const display = this.displayMap.get(id);
        if (!display) return;

        await display.playCutin(group, next_group, name);
    }

    get(id) {
        return this.displayMap.get(id);
    }

    stopAndHideAll() {
        for (const display of this.displayMap.values()) {
            display.stopAll();
        }

        if (this.stopIcon) {
            this.stopIcon.hide();
        }

        this.profile.hide();
    }
}

window.KMCDisplayManager = new DisplayManager();

window.KMCCreateDisplay = (id) => {
    const prefix = (id === 0) ? "player" : "follower";

    // 役割ごとの要素を取得
    const canvases = {
        bg: document.getElementById(`${prefix}_bg`),
        cutin: document.getElementById(`${prefix}_cutin`),
        name: document.getElementById(`${prefix}_name`),
        word: document.getElementById(`${prefix}_word`)
    };

    window.KMCDisplayManager.create(id, canvases);
};

window.KMCDefineCutin = async (json) => {
    window.KMCDisplayManager.initStopIcon();

    const id = json.display_type;

    let display = window.KMCDisplayManager.get(id);
    if (!display) {
        KMCCreateDisplay(id);
        display = window.KMCDisplayManager.get(id);
    }

    const basePath = json.base_path || "";
    const background_path = json.bg_path || null;


    if (!json.entries) return "Error no entries";

    const entryPromises = Object.entries(json.entries).map(async ([key, entry]) => {
        const range = entry.texture_range;
        const dir = basePath + key + "/";
        if (!range) return;

        const start = range.start ?? 0;
        const end = range.end ?? 0;
        const paths = [];
        for (let i = start; i <= end; i++) {
            paths.push(`${dir}${i}.png`);
        }

        const group_id = Number(key.trim());

        return display.defineCutin({
            id: id,
            group: group_id,
            paths: paths,
            layer_name: "CUTIN",
            duration: entry.display_time || 5.0,
            bg_path: background_path || null,
            actor_name: json.actor_name || "",
            word: entry.word || ""
        });
    });

    // 全てのデータ登録（と背景1回のみの読み込み）が完了するのを待つ
    await Promise.all(entryPromises);

    const cache_mode = json.cache_mode ?? 1;
    display.setCacheType(cache_mode);

    // if (cache_mode === 0) {
    //     // 全画像を事前にプリロード(32GB/64GB向け)
    //     console.log(`[CacheMode 0] Bulk Preload Start`);
    //     const all_groups = Object.keys(json.entries).map(key => Number(key.trim()));
    //     await display.bulkPreloadGroups(all_groups);
    // } else if (cache_mode === 1 && json.first_values) {
    //     // 最初のカットイン候補だけプリロード(RAM16GB向け)
    //     console.log(`[CacheMode 1] Partial Bulk Preload`, json.first_values);
    //     await display.bulkPreloadGroups(json.first_values);
    // }

    const loading_overlay = document.getElementById("loading_overlay");
    const progress_bar = document.getElementById("progress_bar");
    const progress_text = document.getElementById("progress_text");

    const update_progress = (percent) => {
        if (progress_bar) progress_bar.style.width = `${percent}%`;
        if (progress_text) progress_text.innerText = `${percent}%`;
    };

    if (cache_mode === 0) {
        // 全画像を事前にプリロード
        console.log(`[CacheMode 0] Bulk Preload Start`);
        if (loading_overlay) loading_overlay.style.display = "block";

        const all_groups = Object.keys(json.entries).map(key => Number(key.trim()));
        await display.bulkPreloadGroupsWithProgress(all_groups, update_progress);

        if (loading_overlay) {
            setTimeout(() => { loading_overlay.style.display = "none"; }, 500);
        }
    } else if (cache_mode === 1 && json.first_values) {
        // 最初のカットイン候補だけプリロード
        console.log(`[CacheMode 1] Partial Bulk Preload`, json.first_values);
        
        await display.bulkPreloadGroupsWithProgress(json.first_values, update_progress);
    }

    return "KMCAddCutinPaths loaded id " + id;
};

window.KMCPreloadGroups = async (json) => {
    const id = Number(json.display_type);
    const groups = json.groups;

    const display = window.KMCDisplayManager.get(id);
    if (!display) return;

    for (const group of groups) {
        await display.preloadGroup(group);
    }
};

window.KMCBatchPreloadGroups = async (json) => {
    for (const [id_str, next_group] of Object.entries(json)) {
        const id = Number(id_str);
        if (id != -1 && next_group != -1) {
            const display_instance = window.KMCDisplayManager.get(id);
            await display_instance.preloadGroup(next_group);
        } else {
            console.warn(`[BatchPreload] Display instance or group not found for ID: ${id}`);
        }
    }
};

window.KMCPlayPlayerCutin = (json) => {
    const group = json.group;
    const next_group = json.next_group;
    const actor_name = json.actor_name;
    window.KMCDisplayManager.play(0, group, next_group, actor_name);
};

window.KMCPlayFollowerCutin = (json) => {
    const id = json.follower_id;
    const group = json.group;
    const next_group = json.next_group;
    const actor_name = json.actor_name;

    if (id == null || group == null) return;

    window.KMCDisplayManager.play(id, group, next_group, actor_name);
};

function makeDraggable(target, handle = target) {
    let isDragging = false;
    let offset = { x: 0, y: 0 };

    handle.style.cursor = "move";
    handle.style.pointerEvents = "auto";

    handle.addEventListener("mousedown", (e) => {
        isDragging = true;
        offset.x = e.clientX - target.offsetLeft;
        offset.y = e.clientY - target.offsetTop;
    });

    window.addEventListener("mousemove", (e) => {
        if (!isDragging) return;

        target.style.left = (e.clientX - offset.x) + "px";
        target.style.top = (e.clientY - offset.y) + "px";
        target.style.right = "auto";
    });

    window.addEventListener("mouseup", () => {
        isDragging = false;
    });
}

makeDraggable(document.getElementById("player_pos"));
makeDraggable(document.getElementById("follower_pos"));

window.KMCShowStopIcon = () => {
    // さすがにプレイヤー＋フォロワーをstopallすると重いので流してしまう
    window.KMCDisplayManager.stopIcon.show();
};

window.KMCHideStopIcon = () => {
    window.KMCDisplayManager.stopIcon.hide();
};

window.KMCStopAndHideCutinAndIcon = () => {
    window.KMCDisplayManager.stopAndHideAll();
};

/**
 * プロフィール
 */
window.KMCSetupProfile = async (json) => {
    await window.KMCDisplayManager.profile.setup(json);
};

window.KMCShowProfile = () => {
    window.KMCDisplayManager.profile.show();
};

window.KMCHideProfile = () => {
    window.KMCDisplayManager.profile.hide();
};

window.KMCUpdateProfileText = (json) => {
    window.KMCDisplayManager.profile.updateText(json);
};