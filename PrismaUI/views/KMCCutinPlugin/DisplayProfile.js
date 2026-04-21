import { DrawUtility } from "./DrawImageUtility.js";

export class DisplayProfile {
    constructor() {
        this.image_cache = new Map();
        this.text_line_elements = new Map();
        this.animating = false;
        this.frame = 0;
        this.animate = this.animate.bind(this);

        this.container = document.getElementById("profile_display");
        
        const ids = ["D01", "D02", "D03"];
        this.ctxs = new Map();
        ids.forEach(id => {
            const canvas = document.getElementById(id);
            if (canvas) {
                canvas.width = canvas.clientWidth || canvas.width;
                canvas.height = canvas.clientHeight || canvas.height;
                this.ctxs.set(id, canvas.getContext("2d"));
            }
        });

        this.text_containers = {};
        ["T01", "T02", "T03", "T04", "T05", "T06"].forEach(id => {
            this.text_containers[id] = document.getElementById(id);
        });
    }

    show() {
        if (this.container) {
            this.container.style.display = "block"; // または contents
        }

        if (!this.animating) {
            this.animating = true;
            requestAnimationFrame(this.animate);
        }
    }

    hide() {
        this.animating = false;
        if (this.container) {
            this.container.style.display = "none";
        }
    }

    animate() {
        if (!this.animating) return;
        requestAnimationFrame(this.animate);

        for (const [id, ctx] of this.ctxs) {
            const frames = this.image_cache.get(id);
            if (!frames || frames.length === 0) continue;

            const bmp = frames[this.frame % frames.length];
            ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);
            DrawUtility.drawImageFit(ctx, bmp, ctx.canvas, "center");
        }
        this.frame++;
    }
}