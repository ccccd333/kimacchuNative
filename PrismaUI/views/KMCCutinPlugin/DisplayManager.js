import { DisplayDrawingTexture } from "./DisplayDrawingTexture.js";
// カットイン関連の大本
// ID = (0)=>プレイヤー (1以降)=>フォロワー
// group = category_id なんでこんな名前にしたんだろ面倒なので今度直す
// TODO:カットインドラッグできるけどドラッグ後に位置保存してないので意味ない。外だし。
export class DisplayManager {
    constructor() {
        this.displayMap = new Map();
    }

    create(id, canvas) {
        const display = new DisplayDrawingTexture(canvas);
        this.displayMap.set(id, display);
    }

    async play(id, group) {
        const display = this.displayMap.get(id);
        if (!display) return;

        await display.playCutin(group);
    }

    get(id) {
        return this.displayMap.get(id);
    }

    stopAllDisplays() {
        for (const display of this.displayMap.values()) {
            display.stopAll();
        }
    }
}

window.KMCDisplayManager = new DisplayManager();

window.KMCCreateDisplay = (id) => {
    const prefix = (id === 0) ? "player" : "follower";

    // 役割ごとの要素を取得
    const canvases = {
        bg: document.getElementById(`${prefix}_bg`),
        cutin: document.getElementById(`${prefix}_cutin`),
        name: document.getElementById(`${prefix}_name`)
    };

    window.KMCDisplayManager.create(id, canvases);
};

window.KMCDefineCutin = async (json) => {
    const id = json.display_type;

    let display = window.KMCDisplayManager.get(id);
    if (!display) {
        KMCCreateDisplay(id);
        display = window.KMCDisplayManager.get(id);
    }

    const basePath = json.base_path || "";
    if (!json.entries) return "Error no entries";


    for (const [key, entry] of Object.entries(json.entries)) {

        const range = entry.texture_range;
        const dir = basePath + key + "/";
        if (!range) continue;

        const start = range.start ?? 0;
        const end = range.end ?? 0;

        const paths = [];

        for (let i = start; i <= end; i++) {
            paths.push(`${dir}${i}.png`);
        }

        const group = Number(key.trim());
        const display_time = entry.display_time ?? 5.0;

        display.defineCutin(paths, id, group, "CUTIN", display_time);
    }

    const cache_mode = json.cache_mode ?? 1;
    display.setCacheType(cache_mode);

    if (cache_mode === 0) {
        // 全画像を事前にプリロード(32GB/64GB向け)
        console.log(`[CacheMode 0] Full Preload for ID: ${id}`);
        const all_groups = Object.keys(json.entries).map(key => Number(key.trim()));

        for (const group_id of all_groups) {
            try {
                await display.preloadGroup(group_id);
            } catch (e) {
                console.error(`Failed to preload: ${group_id}`, e);
            }
        }
    } else if (cache_mode === 1 && json.first_values && Array.isArray(json.first_values)) {
        // 最初のカットイン候補だけプリロード(RAM16GB向け)
        console.log(`[CacheMode 1] Partial Preload for ID: ${id}`, json.first_values);

        for (const group_id of json.first_values) {
            try {
                await display.preloadGroup(group_id);
            } catch (e) {
                console.error(`Failed to preload: ${group_id}`, e);
            }
        }
    }

    return "KMCAddCutinPaths json. loaded id " + id;
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

window.KMCPlayPlayerCutin = (group) => {
    window.KMCDisplayManager.play(0, group);
};

window.KMCPlayFollowerCutin = (json) => {
    const id = json.follower_id;
    const group = json.group;

    if (id == null || group == null) return;

    window.KMCDisplayManager.play(id, group);
};

window.KMCStopAllAnimations = () => {
    // インスタンスのメソッドを呼び出すことで、thisの混乱を防ぐ
    window.KMCDisplayManager.stopAllDisplays();
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