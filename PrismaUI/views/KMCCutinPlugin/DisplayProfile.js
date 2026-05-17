import { DrawUtility } from "./DrawImageUtility.js";

export class DisplayProfile {
    constructor() {
        this.image_cache = new Map();
        this.text_line_elements = new Map();
        this.animating = false;
        this.frame = 0;
        this.animate = this.animate.bind(this);
        this.FPS = 20;
        this.last_time = 0;
        this.interval = 1000 / this.FPS;

        this.container = document.getElementById("profile_display");
        
        this.ctxs = new Map();
        this.text_containers = {};

        this.total_images = 0;
        this.loaded_images = 0;
        
        // const ids = ["D01", "D02", "D03"];
        // this.ctxs = new Map();
        // ids.forEach(id => {
        //     const canvas = document.getElementById(id);
        //     if (canvas) {
        //         canvas.width = canvas.clientWidth || canvas.width;
        //         canvas.height = canvas.clientHeight || canvas.height;
        //         this.ctxs.set(id, canvas.getContext("2d"));
        //     }
        // });

        // this.text_containers = {};
        // ["T01", "T02", "T03", "T04", "T05", "T06"].forEach(id => {
        //     this.text_containers[id] = document.getElementById(id);
        // });
    }

    async setup(json) {
        this.total_images = 0;
        this.loaded_images = 0;

        const tasks = [];
        for (const [id, data] of Object.entries(json)) {
            if (id.startsWith("T")) {
                const el = document.getElementById(id);
                if (el) {
                    this.text_containers[id] = el;
                    this.initTextStructure(id, data);
                }
            } else if (id.startsWith("D")) {
                const canvas = document.getElementById(id);
                if (canvas) {
                    canvas.width = canvas.clientWidth || canvas.width;
                    canvas.height = canvas.clientHeight || canvas.height;
                    this.ctxs.set(id, canvas.getContext("2d"));
                    tasks.push(this.preloadProfile(id, data));
                }
            }
        }

        this.show_profile_progress();

        await Promise.all(tasks);

        this.hide_profile_progress();

        this.container.style.display = "none";

        Object.values(this.text_containers).forEach(el => {
            if (el) el.style.display = "block";
        });

        for (const [id, ctx] of this.ctxs) {
            const el = ctx.canvas.parentElement;
            if (el) el.style.display = "block";
        }
    }

    initTextStructure(id, data) {
        const parent = this.text_containers[id];
        if (!parent) return;

        const target = parent.querySelector(".content") || parent;
        target.innerHTML = "";
        
        const lines = [];
        if (data.lines) {
            data.lines.forEach(text => {
                const div = document.createElement("div");
                div.className = "profile-text-line";
                div.innerText = text;
                target.appendChild(div);
                lines.push(div);
            });
        }
        this.text_line_elements.set(id, lines);

        if (data.bg_path) {
            parent.style.backgroundImage = `url(${data.bg_path})`;
            parent.style.backgroundSize = "cover";
        }
    }

    show_profile_progress() {
        const overlay = document.getElementById("profile_loading_overlay");
        if (overlay) overlay.style.display = "block";
    }

    hide_profile_progress() {
        const overlay = document.getElementById("profile_loading_overlay");
        if (overlay) {
            setTimeout(() => {
                overlay.style.display = "none";
            }, 500);
        }
    }

    update_profile_progress() {
        const percent = this.total_images > 0 
            ? Math.floor((this.loaded_images / this.total_images) * 100) 
            : 0;
        
        const bar = document.getElementById("profile_progress_bar");
        const text = document.getElementById("profile_progress_text");
        
        if (bar) bar.style.width = `${percent}%`;
        if (text) text.innerText = `${percent}%`;
    }

    async preloadProfile(id, data) {
        if (this.image_cache.has(id)) return;

        const { base_path, texture_range, category } = data;
        const dir = base_path + id + "/";
        const category_map = new Map();

        const load_range = async (range, current_dir) => {
            const fetch_promises = [];
            for (let i = range.start; i <= range.end; i++) {
                fetch_promises.push(
                    fetch(`${current_dir}${i}.png`)
                        .then(r => r.blob())
                        .then(blob => createImageBitmap(blob))
                        .then(bmp => {
                            this.loaded_images++;
                            this.update_profile_progress();
                            return bmp;
                        })
                        .catch(e => {
                            console.error(`[Profile] Load fail: ${id}_${i}`, e);
                            this.loaded_images++;
                            this.update_profile_progress();
                            return null;
                        })
                );
            }
            const bitmaps = await Promise.all(fetch_promises);
            return bitmaps.filter(b => b);
        };

        this.total_images += (texture_range.end - texture_range.start + 1);

        if (category) {
            for (const [cat_name, cat_data] of Object.entries(category)) {
                if (cat_data.texture_range) {
                    this.total_images += (cat_data.texture_range.end - cat_data.texture_range.start + 1);
                }
            }
        }

        const default_bitmaps = await load_range(texture_range, dir);
        category_map.set("default", default_bitmaps);

        if (category) {
            for (const [cat_name, cat_data] of Object.entries(category)) {
                if (cat_data.texture_range) {
                    const cat_dir = dir + cat_name + "/";
                    const cat_bitmaps = await load_range(cat_data.texture_range, cat_dir);
                    category_map.set(cat_name, cat_bitmaps);
                }
            }
        }

        this.image_cache.set(id, category_map);
    }

    show(category) {
        this.current_category = category;
        this.container.style.display = "block";

        console.log(`[DisplayProfile] show() called with category: "${this.current_category}"`);

        if (!this.animating) {
            this.animating = true;

            this.last_time = 0;
            this.frame = 0;

            requestAnimationFrame(this.animate);
        }
    }

    hide() {
        this.animating = false;
        this.container.style.display = "none";
    }

    animate(time) {
        if (!this.animating) return;
        requestAnimationFrame(this.animate);

        const elapsed = time - this.last_time;

        if (elapsed < this.interval) return;

        this.last_time = time - (elapsed % this.interval);

        for (const [id, ctx] of this.ctxs) {
            const category_map = this.image_cache.get(id);
            if (!category_map) continue;

            let frames = category_map.get(this.current_category);
            if (!frames || frames.length === 0) {
                frames = category_map.get("default");
                console.log(`[DisplayProfile] Category "${this.current_category}" not found for ${id}, fallback to "default"`);
            }
            
            if (!frames || frames.length === 0) {
                console.warn(`[DisplayProfile] No frames available for ${id}`);
                continue
            };

            const bmp = frames[this.frame % frames.length];
            ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);
            DrawUtility.drawImageFit(ctx, bmp, ctx.canvas, "center");
        }
        this.frame++;
    }

    updateText(json) {
        for (const [id, lines] of Object.entries(json)) {
            const lineElements = this.text_line_elements.get(id);
            if (!lineElements) continue;

            for (const [index, text] of Object.entries(lines)) {
                const el = lineElements[index];
                if (el) {
                    el.innerText = text;
                }
            }
        }
    }
}