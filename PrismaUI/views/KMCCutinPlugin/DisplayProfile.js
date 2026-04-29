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

        await Promise.all(tasks);

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

    async preloadProfile(id, data) {
        if (this.image_cache.has(id)) return;
        const { base_path, texture_range } = data;
        const fetchPromises = [];
        const dir = base_path + id + "/";
        for (let i = texture_range.start; i <= texture_range.end; i++) {
            fetchPromises.push(
                fetch(`${dir}${i}.png`)
                    .then(r => r.blob())
                    .then(blob => createImageBitmap(blob))
                    .catch(e => console.error(`[Profile] Load fail: ${id}_${i}`, e))
            );
        }
        const bitmaps = await Promise.all(fetchPromises);
        this.image_cache.set(id, bitmaps.filter(b => b));
    }

    show() {
        this.container.style.display = "block";

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
            const frames = this.image_cache.get(id);
            if (!frames || frames.length === 0) continue;

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