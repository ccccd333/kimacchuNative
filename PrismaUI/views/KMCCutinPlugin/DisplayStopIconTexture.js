import { DrawUtility } from "./DrawImageUtility.js";

// システム共通の停止アイコン管理クラス
export class DisplayStopIconTexture {
    constructor(canvas) {
        this.canvas = canvas;
        this.ctx = canvas ? canvas.getContext("2d") : null;
        this.icon_bitmap = null;

        if (this.canvas) {
            this.canvas.width = this.canvas.clientWidth;
            this.canvas.height = this.canvas.clientHeight;
        }

        this.loadIcon();
    }

    // アイコン画像の読み込み
    async loadIcon() {
        try {
            const res = await fetch("stop_icon.png");
            const blob = await res.blob();
            this.icon_bitmap = await createImageBitmap(blob);
            console.log("Global stop icon loaded.");
        } catch (e) {
            console.error("Failed to load global stop icon:", e);
        }
    }

    show() {
        if (!this.icon_bitmap || !this.ctx) return;

        const parent = this.canvas.parentElement;
        if (parent) {
            this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
            DrawUtility.drawImageFit(this.ctx, this.icon_bitmap, this.canvas, "center");

            parent.classList.add("animating-pulse");
        }
    }

    hide() {
        const parent = this.canvas.parentElement;
        if (parent) {
            parent.classList.remove("animating-pulse");
            parent.style.opacity = "0";
        }
    }
}