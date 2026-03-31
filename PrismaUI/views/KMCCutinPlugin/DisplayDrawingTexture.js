import { CutinData } from "./CutinData.js";
import { DrawUtility } from "./DrawImageUtility.js";

export class DisplayDrawingTexture {
    constructor(canvases) {
        this.cutin_map = new Map();
        
        this.canvases = canvases;

        this.ctxs = {
            bg: canvases.bg.getContext("2d"),
            cutin: canvases.cutin.getContext("2d"),
            name: canvases.name.getContext("2d")
        };

        Object.values(this.canvases).forEach(canvas => {
            if (canvas) {
                canvas.width = canvas.clientWidth;
                canvas.height = canvas.clientHeight;
            }
        });

        this.bitmaps = [];
        this.frame = 0;
        this.lastTime = 0;
        this.FPS = 24;
        this.interval = 1000 / this.FPS;
        this.animating = false;
        this.animate = this.animate.bind(this);

        this.currentGroup = null;
        this.duration_sec = 5.0;
        this.display_type = 0;
        this.startTime = null;
    }

    defineCutin(paths, id, group, layerName, duration = 5.0) {
        if (!this.cutin_map.has(group)) {
            this.cutin_map.set(group, new CutinData());
        }

        const cutin = this.cutin_map.get(group);

        if (!cutin.layers.has(layerName)) {
            cutin.layers.set(layerName, []);
        }

        cutin.layers.get(layerName).push(...paths);
        cutin.duration = duration;
        cutin.display_type = id;

        let total = 0;
        for (const arr of cutin.layers.values()) {
            total += arr.length;
        }
        cutin.single = total === 1;
    }

    async preloadGroup(group) {
        if (!this.cache) this.cache = new Map();

        if (this.cache.has(group)) {
            return this.cache.get(group);
        }

        const promise = (async () => {
            const cutin = this.cutin_map.get(group);
            if (!cutin) return [];

            // CUTINレイヤーの画像のみをプリロード対象
            const paths = cutin.layers.get("CUTIN") || [];
            const frames = new Array(paths.length);

            const concurrency = 4;
            let index = 0;

            const worker = async () => {
                while (true) {
                    const i = index++;
                    if (i >= paths.length) break;

                    try {
                        const res = await fetch(paths[i]);
                        const blob = await res.blob();
                        const bmp = await createImageBitmap(blob);
                        frames[i] = bmp;
                    } catch (e) {
                        console.error("Failed to load texture:", paths[i], e);
                    }
                }
            };

            const workers = Array.from({ length: concurrency }, () => worker());
            await Promise.all(workers);

            return frames;
        })();

        this.cache.set(group, promise);
        return promise;
    }

    async playCutin(group) {
        const frames = await this.preloadGroup(group);
        if (!frames || frames.length === 0) return;

        const cutin = this.cutin_map.get(group);

        this.bitmaps = frames;
        this.frame = 0;
        this.lastTime = 0;
        this.startTime = null;
        this.animating = true;

        this.currentGroup = group;
        this.duration_sec = cutin?.duration ?? 5.0;
        this.display_type = cutin.display_type;

        // 全レイヤーを表示状態にする
        Object.values(this.canvases).forEach(canvas => {
            if (canvas) canvas.style.display = "block";
        });

        requestAnimationFrame(this.animate);
    }

    animate(time) {
        if (!this.animating) return;

        requestAnimationFrame(this.animate);

        if (this.startTime === null) {
            this.startTime = time;
        }

        if (time - this.lastTime < this.interval) return;
        this.lastTime = time;

        // カットイン層のみをクリアして描画
        const ctx_cutin = this.ctxs.cutin;
        const canvas_cutin = this.canvases.cutin;
        
        if (ctx_cutin && canvas_cutin) {
            ctx_cutin.clearRect(0, 0, canvas_cutin.width, canvas_cutin.height);

            const bmp = this.bitmaps[this.frame];
            if (bmp) {
                DrawUtility.drawImageFit(ctx_cutin, bmp, canvas_cutin, "");
            }
        }

        this.frame = (this.frame + 1) % this.bitmaps.length;

        // 終了判定
        if ((time - this.startTime) >= this.duration_sec * 1000) {
            this.animating = false;

            // 全てのレイヤーを非表示にする
            Object.keys(this.ctxs).forEach(key => {
                const ctx = this.ctxs[key];
                const canvas = this.canvases[key];
                if (ctx && canvas) {
                    ctx.clearRect(0, 0, canvas.width, canvas.height);
                    canvas.style.display = "none";
                }
            });

            if (window.KMCOnCutinFinished) {
                window.KMCOnCutinFinished(this.display_type);
            }
            return;
        }
    }
}

window.KMCOnCutinFinished = (display_type) => {
    console.log("Cutin finished for display:", display_type);
};