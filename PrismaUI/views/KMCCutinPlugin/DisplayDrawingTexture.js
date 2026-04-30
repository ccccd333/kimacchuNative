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
            name: canvases.name.getContext("2d"),
            word: canvases.word.getContext("2d")
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
        this.FPS = 20;
        this.interval = 1000 / this.FPS;
        this.animating = false;
        this.animate = this.animate.bind(this);

        // iwant widget時代はC++で制御してたけど
        // prismaからはJS側で画像再生→終了通知が自然だ
        this.current_group = null;
        this.current_next_group = null;
        this.duration_sec = 5.0;
        this.display_type = 0;
        this.start_time = null;
        this.cutin_word = "";

        // (1)=>部分読み込みモード、(0)=>全画像読み込みモード
        this.cache_type = 1;

        // プレイヤー名はshow racemenuとかで変更できるからと思ったけどどうしよ
        this.actor_name = "";
        // カットインの背景
        this.bg_path = "";
        this.bg_bitmap = null;
        this.bg_loading = false;
        // anim中のフォント設定
        this.actor_font_spec = "";
        this.actor_fill_color = "";
        this.actor_shadow_color = "";

        this.word_font_spec = "";
        this.word_fill_color = "";
        this.word_shadow_color = "";

        this.word_line_height = 24;

        this.font = "sans-serif";
    }

    setCacheType(chache_type) {
        this.cache_type = chache_type;
    }

    async defineCutin(config) {
        const {
            id,
            group,
            paths,
            layer_name = "CUTIN",
            duration = 5.0,
            bg_path = null,
            actor_name = "",
            word = "",
            font,
            actor_size, actor_bold, actor_fill, actor_shadow,
            word_size, word_bold, word_fill, word_shadow,
        } = config;

        this.display_type = id;
        this.actor_name = actor_name;
        this.font = font;
        if (!this.cutin_map.has(group)) {
            this.cutin_map.set(group, new CutinData());
        }

        const cutin = this.cutin_map.get(group);

        if (!cutin.layers.has(layer_name)) {
            cutin.layers.set(layer_name, []);
        }

        cutin.layers.get(layer_name).push(...paths);

        //cutin.actor_name = actor_name;
        cutin.word = word;

        cutin.display_time = duration;

        const boldStr = (isBold) => isBold ? "bold" : "";

        cutin.actor_font_spec = `${boldStr(actor_bold)} ${actor_size}px ${this.font}`.trim();
        cutin.actor_fill_color = actor_fill;
        cutin.actor_shadow_color = actor_shadow;

        cutin.word_font_spec = `${boldStr(word_bold)} ${word_size}px ${this.font}`.trim();
        cutin.word_fill_color = word_fill;
        cutin.word_shadow_color = word_shadow;

        cutin.word_line_height = word_size + 4;


        let total = 0;
        for (const arr of cutin.layers.values()) {
            total += arr.length;
        }
        cutin.single = total === 1;

        if (bg_path && !this.bg_bitmap && !this.bg_loading) {
            this.bg_loading = true;
            this.bg_path = bg_path;
            try {
                const res = await fetch(this.bg_path);
                const blob = await res.blob();
                this.bg_bitmap = await createImageBitmap(blob);
                console.log(`Background loaded: ${this.bg_path}`);
            } catch (e) {
                console.error("Failed to load background:", e);
            } finally {
                this.bg_loading = false;
            }

            // 初回表示時カクつくので仮描画
            Object.values(this.canvases).forEach(canvas => {
                if (canvas) canvas.style.display = "block";
            });

            const res = await fetch("test.png");
            const blob = await res.blob();
            const dummy_bmp = await createImageBitmap(blob);

            DrawUtility.drawImageFit(this.ctxs.bg, dummy_bmp, this.canvases.bg, "");

            const ctx_cutin = this.ctxs.cutin;
            const canvas_cutin = this.canvases.cutin;
            DrawUtility.drawImageFit(ctx_cutin, dummy_bmp, canvas_cutin, "");


            const ctx = this.ctxs.name;
            ctx.fillStyle = cutin.actor_fill_color;
            ctx.font = cutin.actor_font_spec;
            ctx.shadowColor = cutin.actor_shadow_color;
            ctx.shadowBlur = 4;
            ctx.fillText(this.actor_name, 10, 30);


            const ctx2 = this.ctxs.word;
            ctx2.fillStyle = cutin.word_fill_color;
            ctx2.font = cutin.word_font_spec;
            ctx2.shadowColor = cutin.word_shadow_color;
            ctx2.shadowBlur = 4;
            ctx2.fillText("", 10, 30);

            Object.keys(this.ctxs).forEach(key => {
                const ctx = this.ctxs[key];
                const canvas = this.canvases[key];
                if (ctx && canvas) {
                    ctx.clearRect(0, 0, canvas.width, canvas.height);
                    canvas.style.display = "none";
                }
            });

            dummy_bmp.close();

            const lines = word.split('\n');
            const line_widths = lines.map(line => this.ctxs.word.measureText(line).width);
            const max_line_width = Math.max(...line_widths);
            cutin.word_lines = lines;
            cutin.word_start_x = (this.canvases.word.width - max_line_width) / 2;
            cutin.word_total_height = lines.length * cutin.word_line_height;

        }
    }

    async bulkPreloadGroupsWithProgress(group_ids, on_progress) {
        if (!this.cache) this.cache = new Map();

        const all_paths_to_load = [];
        const group_assignments = [];

        for (const raw_id of group_ids) {
            const group_id = Number(raw_id);
            if (this.cache.has(group_id)) continue;

            const cutin_data = this.cutin_map.get(group_id);
            if (!cutin_data) continue;

            const paths = cutin_data.layers.get("CUTIN") || [];
            if (paths.length === 0) continue;

            const start_idx = all_paths_to_load.length;
            all_paths_to_load.push(...paths);
            const end_idx = all_paths_to_load.length;

            group_assignments.push({
                id: group_id,
                start: start_idx,
                end: end_idx
            });
        }

        const total_images = all_paths_to_load.length;
        if (total_images === 0) {
            if (on_progress) on_progress(100);
            return;
        }

        let loaded_count = 0;

        // 全パスに対して一斉に非同期処理を開始 上限なし
        const load_tasks = all_paths_to_load.map(async (path) => {
            try {
                const response = await fetch(path);
                const blob = await response.blob();
                const bitmap = await createImageBitmap(blob);
                return bitmap;
            } catch (error) {
                console.error(`[Bulk] Load failed: ${path}`, error);
                return null;
            } finally {
                loaded_count++;
                if (on_progress) {
                    const percent = Math.floor((loaded_count / total_images) * 100);
                    on_progress(percent);
                }
            }
        });

        const all_bitmaps = await Promise.all(load_tasks);

        for (const assign of group_assignments) {
            const group_bitmaps = all_bitmaps.slice(assign.start, assign.end);
            this.cache.set(assign.id, Promise.resolve(group_bitmaps));
        }

        console.info(`[Bulk Preload] Completed: ${total_images} images (Unlimited Parallel).`);
    }

    async bulkPreloadGroups(group_ids) {
        if (!this.cache) this.cache = new Map();

        const allTasks = [];

        for (const raw_id of group_ids) {
            const group = Number(raw_id);
            if (this.cache.has(group)) continue;

            const cutin = this.cutin_map.get(group);
            if (!cutin) continue;

            const paths = cutin.layers.get("CUTIN") || [];
            if (paths.length === 0) continue;

            const loadPromises = paths.map(async (path) => {
                try {
                    const res = await fetch(path);
                    const blob = await res.blob();
                    return await createImageBitmap(blob);
                } catch (e) {
                    console.error(`[Bulk] Load failed: ${path}`, e);
                    return null;
                }
            });

            const groupPromise = Promise.all(loadPromises);

            this.cache.set(group, groupPromise);
            allTasks.push(groupPromise);
        }

        console.info(`[Bulk Preload] Simultaneous launch for ${allTasks.length} groups.`);

        await Promise.all(allTasks);

        console.info(`[Bulk Preload] All images are now in memory.`);
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

    async playCutin(group, next_group, actor_name) {
        if (this.animating) {
            console.warn(`Already animating. Ignore play request for group: ${group}`);
            return;
        }

        const frames = await this.preloadGroup(group);

        if (!frames || frames.length === 0) {
            OnCutinUnavailable(this.display_type);
            return;
        }
        const cutin = this.cutin_map.get(group);

        this.bitmaps = frames;
        this.frame = 0;
        this.last_time = 0;
        this.start_time = null;
        this.animating = true;
        this.current_group = group;
        this.current_next_group = next_group;
        this.duration_sec = cutin?.display_time ?? 5.0;
        this.cutin_word = cutin?.word ?? "";
        this.actor_name = actor_name;

        this.actor_font_spec = cutin.actor_font_spec;
        this.actor_fill_color = cutin.actor_fill_color;
        this.actor_shadow_color = cutin.actor_shadow_color;

        this.word_font_spec = cutin.word_font_spec;
        this.word_fill_color = cutin.word_fill_color;
        this.word_shadow_color = cutin.word_shadow_color;


        this.word_lines = cutin.word_lines;
        this.word_start_x = cutin.word_start_x;
        this.word_total_height = cutin.word_total_height;
        this.word_line_height = cutin.word_line_height;


        // CPUレンダリングなのでフェードとかスンナとのことなのでスライドアニメーション
        const unit_container = this.canvases.cutin.parentElement;
        if (unit_container) {
            unit_container.classList.remove("slide-in");
            void unit_container.offsetWidth;
            unit_container.classList.add("slide-in");
        }

        // 全レイヤーを表示状態にする
        Object.values(this.canvases).forEach(canvas => {
            if (canvas) canvas.style.display = "block";
        });

        requestAnimationFrame(this.animate);

        // カットインできたよ通知
        // c++側と音声タイミングがずれるようだったら位置変える
        OnCutinStartReady(this.display_type);
    }

    animate(time) {
        if (!this.animating) return;

        requestAnimationFrame(this.animate);

        if (this.start_time === null) {
            // 初回時のみ描画するもの
            if (this.bg_bitmap && this.ctxs.bg) {
                //this.ctxs.bg.clearRect(0, 0, this.canvases.bg.width, this.canvases.bg.height);
                DrawUtility.drawImageFit(this.ctxs.bg, this.bg_bitmap, this.canvases.bg, "center");
            }

            if (this.ctxs.name) {
                const ctx = this.ctxs.name;
                const canvas = this.canvases.name;
                ctx.clearRect(0, 0, canvas.width, canvas.height);

                ctx.font = this.actor_font_spec;
                ctx.fillStyle = this.actor_fill_color;
                ctx.shadowColor = this.actor_shadow_color;
                ctx.shadowBlur = 4;

                ctx.textAlign = "center";

                ctx.fillText(this.actor_name, canvas.width / 2, 35);
            }

            if (this.ctxs.word) {
                const ctx = this.ctxs.word;
                ctx.clearRect(0, 0, this.canvases.word.width, this.canvases.word.height);

                ctx.font = this.word_font_spec; // エントリーごとの設定を適用
                ctx.fillStyle = this.word_fill_color;
                ctx.shadowColor = this.word_shadow_color;
                ctx.shadowBlur = 4;

                ctx.textAlign = "left";
                
                let y = (this.canvases.word.height - this.word_total_height) / 2 + (this.word_line_height / 1.5);

                for (const line of this.word_lines) {
                    ctx.fillText(line, this.word_start_x, y);
                    y += this.word_line_height;
                }
            }

            this.start_time = time;
        }

        if (time - this.last_time < this.interval) return;
        this.last_time = time;

        // インターバル毎に描画するもの
        const ctx_cutin = this.ctxs.cutin;
        const canvas_cutin = this.canvases.cutin;

        if (ctx_cutin && canvas_cutin) {
            //ctx_cutin.clearRect(0, 0, canvas_cutin.width, canvas_cutin.height);

            const bmp = this.bitmaps[this.frame];
            if (bmp) {
                DrawUtility.drawImageFit(ctx_cutin, bmp, canvas_cutin, "center");
            }
        }


        this.frame = (this.frame + 1) % this.bitmaps.length;

        // アニメーション秒数を超えた場合C++に通知とキャッシュモード(1)の場合は再生済みは消す
        if ((time - this.start_time) >= this.duration_sec * 1000) {
            this.animating = false;

            // 全てのレイヤー非表示
            Object.keys(this.ctxs).forEach(key => {
                const ctx = this.ctxs[key];
                const canvas = this.canvases[key];
                if (ctx && canvas) {
                    ctx.clearRect(0, 0, canvas.width, canvas.height);
                    canvas.style.display = "none";
                }
            });

            if (this.cache_type === 1 && this.current_next_group != this.current_group) {
                // current_next_groupが-1の場合はフォロワーの場合ではあり得る。
                // プレイヤーをベースにカットインIDが決定するため、プレイヤーの場合ここに来る条件に-1の場合はないが
                // プレイヤーには存在するカットインIDで、フォロワー側には定義がない場合があるため。
                // これはIDで一致するプレイヤー→フォロワーで順番にカットインする仕様によるもの

                const group_promise = this.cache.get(this.current_group);
                if (group_promise) {
                    group_promise.then(frames => {
                        frames.forEach(bmp => bmp.close());
                    });
                    this.cache.delete(this.current_group);
                    console.log(`Released group: ${this.current_group}`);
                }


                if (this.current_next_group != -1) {
                    this.preloadGroup(this.current_next_group);
                }

                console.log(`preloadGroup loaded. ${this.current_group} ${this.current_next_group}`);
            }

            this.current_next_group = -1;

            // C++側に通知
            CutinFinished(this.display_type);
            return;
        }
    }

    stopAll() {
        // ゲームロード時
        if (!this.animating) return;
        this.animating = false;

        // TODO:停止アイコン表示時stopするので、next_group -1はあり得るか(要テスト)
        if (this.cache_type === 1 && this.current_next_group && this.current_group &&
            this.current_next_group != this.current_group
        ) {
            const group_promise = this.cache.get(this.current_group);
            if (group_promise) {
                group_promise.then(frames => {
                    frames.forEach(bmp => bmp.close());
                });
                this.cache.delete(this.current_group);
                console.log(`Released group: ${this.current_group}`);
            }

            if (this.current_next_group != -1) {
                this.preloadGroup(this.current_next_group);
            }
        }

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

    }
}

function CutinFinished(display_type) {
    window.KMCOnCutinFinished(display_type);
    addResponse(display_type);
}

function OnCutinStartReady(display_type) {
    window.KMCOnCutinStartReady(display_type);
    addResponse(display_type);
}

function OnCutinUnavailable(display_type) {
    window.KMCOnCutinUnavailable(display_type);
    addResponse(display_type);
}