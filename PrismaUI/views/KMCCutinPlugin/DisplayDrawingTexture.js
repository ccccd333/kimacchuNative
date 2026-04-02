import { CutinData } from "./CutinData.js";
import { DrawUtility } from "./DrawImageUtility.js";
// カットインの個別(プレイヤー、フォロワー)管理クラス
// 再生対象の管理と再生サイクル
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

        // TODO:多分、大体動画生成やGIFって24FPSだったような気がする
        // そのうち外部から送るようにする。それより開発PC壊れそうなのでTODO
        this.bitmaps = [];
        this.frame = 0;
        this.last_time = 0;
        this.FPS = 24;
        this.interval = 1000 / this.FPS;
        this.animating = false;
        this.animate = this.animate.bind(this);

        // iwant widget時代はC++で制御してたけど
        // prismaからはJS側で画像再生→終了通知が自然だ
        this.current_group = null;
        this.duration_sec = 5.0;
        this.display_type = 0;
        this.start_time = null;
        // (1)=>部分読み込みモード、(0)=>全画像読み込みモード
        this.cache_type = 1;
    }

    setCacheType(chache_type) {
        this.cache_type = chache_type;
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
        if (this.animating) {
            console.warn(`Already animating. Ignore play request for group: ${group}`);
            return;
        }

        const frames = await this.preloadGroup(group);
        if (!frames || frames.length === 0) return;

        const cutin = this.cutin_map.get(group);

        this.bitmaps = frames;
        this.frame = 0;
        this.last_time = 0;
        this.start_time = null;
        this.animating = true;

        // CPUレンダリングなのでフェードとかスンナとのことなのでスライドアニメーション
        const unit_container = this.canvases.cutin.parentElement;
        if (unit_container) {
            unit_container.classList.remove("slide-in");
            void unit_container.offsetWidth;
            unit_container.classList.add("slide-in");
        }

        this.current_group = group;
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

        if (this.start_time === null) {
            this.start_time = time;
        }

        if (time - this.last_time < this.interval) return;
        this.last_time = time;

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

        // アニメーション秒数を超えた場合C++に通知とキャッシュモード(1)の場合は再生済みは消す
        if ((time - this.start_time) >= this.duration_sec * 1000) {
            this.animating = false;

            if (this.cache_type === 1) {
                const group_promise = this.cache.get(this.current_group);
                if (group_promise) {
                    group_promise.then(frames => {
                        frames.forEach(bmp => bmp?.close?.());
                    });
                    this.cache.delete(this.current_group);
                    console.log(`Released group: ${this.current_group}`);
                }
            }

            // 全てのレイヤー非表示
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

    stopAll() {
        this.animating = false;

        // ビットマップの配列を空にするだけで、各bmpのcloseは呼ばない
        // キャッシュ側にあるデータは生存し続ける
        // なんかGCに残りすぎると若干気持ち悪い、切り替えは早いけど
        // PrismaUIがCPUレンダリングなのでRAMに乗りっぱなし
        // 150*150.pngなら見た感じOK、フルHDはできるだけ残したくないけど読み込みおっそいからしょうがない
        this.bitmaps = [];
        this.current_group = null;

        Object.keys(this.ctxs).forEach(key => {
            const ctx = this.ctxs[key];
            const canvas = this.canvases[key];
            if (ctx && canvas) {
                ctx.clearRect(0, 0, canvas.width, canvas.height);
                canvas.style.display = "none";
            }
        });
        console.log(`Animation stopped. Bitmaps retained in cache for ID: ${this.display_type}`);
    }
}

window.KMCOnCutinFinished = (display_type) => {
    console.log("Cutin finished for display:", display_type);
};