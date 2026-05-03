import { Texture } from "./Texture.js";
import { DrawUtility } from "./DrawImageUtility.js";

export class DisplayDrawingTexture {
    constructor(canvas) {
        this.texture_map = new Map();
        this.canvas = canvas;
        this.ctx = canvas.getContext("2d");
        this.canvas.width = canvas.clientWidth;
        this.canvas.height = canvas.clientHeight;

        this.bitmaps = [];
        this.frame = 0;
        this.lastTime = 0;
        this.FPS = 24;
        this.interval = 1000 / this.FPS;
        this.animating = false;
        this.animate = this.animate.bind(this);
    }

    addPaths(paths, group,layerName) {
        for (const path of paths) {
            const parts = path.split("/");

            if (!this.texture_map.has(group)) {
                this.texture_map.set(group, new Texture());
            }

            const tex = this.texture_map.get(group);

            if (!tex.layers.has(layerName)) {
                tex.layers.set(layerName, []);
            }

            tex.layers.get(layerName).push(path);
        }

        for (const tex of this.texture_map.values()) {
            let total = 0;
            for (const arr of tex.layers.values()) {
                total += arr.length;
            }
            tex.single = total === 1;
        }
    }

    async loadFrames(group) {
        const texture = this.texture_map.get(group);
        if (!texture) return;

        // 各レイヤー取得
        const bgPaths = texture.layers.get("BACKGROUND") || [];
        const namePaths = texture.layers.get("NAME") || [];
        const cutinPaths = texture.layers.get("CUTIN") || [];

        // 初期化
        this.bgBitmap = null;
        this.nameBitmap = null;
        this.bitmaps = new Array(cutinPaths.length);

        // 背景・名前（軽いので直列）
        if (bgPaths[0]) {
            const res = await fetch(bgPaths[0]);
            this.bgBitmap = await createImageBitmap(await res.blob());
        }

        if (namePaths[0]) {
            const res = await fetch(namePaths[0]);
            this.nameBitmap = await createImageBitmap(await res.blob());
        }

    this.bitmaps = new Array(cutinPaths.length);
    this.frame = 0;
    this.animating = false;

    const concurrency = 4;

    let index = 0;

    const worker = async () => {
        while (true) {
            const i = index++;
            if (i >= cutinPaths.length) break;

            const path = cutinPaths[i];

            const res = await fetch(path);
            const blob = await res.blob();
            const bmp = await createImageBitmap(blob);

            this.bitmaps[i] = bmp;
        }
    };

    const workers = [];
    for (let i = 0; i < concurrency; i++) {
        workers.push(worker());
    }

    // 全部ロード終わるの待つ
    await Promise.all(workers);

    // 最後に再生開始
    if (this.bitmaps.length > 0) {
        this.animating = true;
        requestAnimationFrame(this.animate);
    }

        // // 並列ロード
        // const promises = cutinPaths.map((path, i) =>
        //     fetch(path)
        //         .then(res => res.blob())
        //         .then(blob => createImageBitmap(blob))
        //         .then(bmp => {
        //             this.bitmaps[i] = bmp;

        //             // 最初の1枚が来たら再生開始
        //             if (!this.animating) {
        //                 this.animating = true;
        //                 requestAnimationFrame(this.animate);
        //             }
        //         })
        // );

    }

    animate(time) {
        requestAnimationFrame(this.animate);

        if (time - this.lastTime < this.interval) return;
        this.lastTime = time;

        const ctx = this.ctx;
        ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);

        if (this.bgBitmap) {
            DrawUtility.drawImageFit(ctx, this.bgBitmap, this.canvas, "center");
        }

        if (this.nameBitmap) {
            DrawUtility.drawImageFit(ctx, this.nameBitmap, this.canvas, "top-left");
        }

        if (!this.bitmaps || this.bitmaps.length === 0) return;

        const bmp = this.bitmaps[this.frame];

        if (!bmp) return;

        DrawUtility.drawImageFit(ctx, bmp, this.canvas, "top-right");

        this.frame = (this.frame + 1) % this.bitmaps.length;
    }


}

window.KMCAddCutinPaths = (json) => {
    display.addPaths(json, "CUTIN")
};