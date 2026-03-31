export const DrawUtility = {

    drawImageFit(ctx, img, canvas, align) {

        const scale = Math.min(
            canvas.width / img.width,
            canvas.height / img.height
        );

        const scale_tr = Math.min(
            (canvas.width / 2) / img.width,
            (canvas.height / 2) / img.height
        );

        let w, h;

        // サイズ決定
        if (align === "corner-top-left" || align === "corner-top-right" ||
            align === "corner-bottom-left" || align === "corner-bottom-right") {
            w = img.width * scale_tr;
            h = img.height * scale_tr;
        } else {
            w = img.width * scale;
            h = img.height * scale;
        }

        let x = 0;
        let y = 0;

        // 位置決定
        if (align === "corner-top-left") {
            x = 0;
            y = 0;
        } else if (align === "corner-top-right") {
            x = canvas.width - w;
            y = 0;
        } else if (align === "corner-bottom-left") {
            x = 0;
            y = canvas.height - h;
        } else if (align === "corner-bottom-right") {
            x = canvas.width - w;
            y = canvas.height - h;
        } else if (align === "center") {
            x = (canvas.width - w) / 2;
            y = (canvas.height - h) / 2;
        }

        ctx.drawImage(img, x, y, w, h);

    }
};