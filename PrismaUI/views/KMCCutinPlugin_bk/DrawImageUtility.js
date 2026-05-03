export const DrawUtility = {

    drawImageFit(ctx, img, canvas, align) {
        const scale = Math.min(
            canvas.width / img.width,
            canvas.height / img.height
        );

        const w = img.width * scale;
        const h = img.height * scale;

        let x = 0;
        let y = 0;

        if (align === "top-left") {
            x = 0;
            y = 0;
        } else if (align === "top-right") {
            x = canvas.width - w;
            y = 0;
        } else if (align === "center") {
            x = (canvas.width - w) / 2;
            y = (canvas.height - h) / 2;
        }

        ctx.drawImage(img, x, y, w, h);

    }
};