import { DisplayDrawingTexture } from "./DisplayDrawingTexture.js";

export class DisplayManager {
    constructor() {
        this.displayMap = new Map();
    }

    create(id, canvas) {
        const display = new DisplayDrawingTexture(canvas);
        this.displayMap.set(id, display);
    }

    addPaths(id, paths) {
        const display = this.displayMap.get(id);
        if (!display) return;

        display.addPaths(paths, "CUTIN");
    }

    load(id, group) {
        const display = this.displayMap.get(id);
        if (!display) return;

        display.loadFrames(group);
    }

    get(id) {
        return this.displayMap.get(id);
    }
}

window.KMCDisplayManager = new DisplayManager();

window.KMCCreateDisplay = (id) => {
    let elem_name = "player_canvas";

    if (id > 0){
        elem_name = "follower_canvas";
    }

    const canvas = document.getElementById(elem_name);
    window.KMCDisplayManager.create(id, canvas);
};

window.KMCAddCutinPaths = (json) => {
    const id = json.display_type;

    let display = window.KMCDisplayManager.get(id);
    if (!display){ 
        KMCCreateDisplay(id);
        display = window.KMCDisplayManager.get(id);
    }

    const basePath = json.base_path || "";

    if (!json.entries) return "Error no entries";

    for (const [key, entry] of Object.entries(json.entries)) {

        const range = entry.texture_range;
        const dir = basePath + key + "/"
        if (!range) continue;

        const start = range.start ?? 0;
        const end = range.end ?? 0;

        const paths = [];

        for (let i = start; i <= end; i++) {
            paths.push(`${dir}${i}.png`);
            console.log(`${dir}${i}.png`);
        }

        const group = Number(key.trim());

        display.addPaths(paths, group, "CUTIN");
    }

    return "KMCAddCutinPaths json. loaded id " + id;
};

window.KMCPlayPlayerCutin = (group) => {
    window.KMCDisplayManager.load(0, group);
};

window.KMCPlayFollowerCutin = (json) => {
    const id = json.follower_id;
    const group = json.group;

    if (id == null || group == null) return;

    window.KMCDisplayManager.load(id, group);
};