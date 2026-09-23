import QtQuick

// Small self-drawn glyph set (bell, sun/moon, info, play, record, chevron,
// warning, check) so the app needs no bundled icon assets. Mirrors the
// reference design's "single glyph, recolor at runtime" pattern - here done
// with a tinted Canvas path instead of a CSS/QML mask on a bitmap.
Canvas {
    id: root
    property string name: "info"
    property color color: "#ffffff"
    width: 24
    height: 24

    onNameChanged: requestPaint()
    onColorChanged: requestPaint()
    onWidthChanged: requestPaint()

    onPaint: {
        var ctx = getContext("2d");
        ctx.reset();
        ctx.strokeStyle = color;
        ctx.fillStyle = color;
        ctx.lineWidth = Math.max(1.5, width * 0.09);
        ctx.lineCap = "round";
        ctx.lineJoin = "round";
        var w = width, h = height, cx = w / 2, cy = h / 2;

        switch (name) {
        case "bell":
            ctx.beginPath();
            ctx.arc(cx, cy - h * 0.05, w * 0.28, Math.PI, 0, false);
            ctx.lineTo(w * 0.78, h * 0.62);
            ctx.lineTo(w * 0.22, h * 0.62);
            ctx.closePath();
            ctx.fill();
            ctx.beginPath();
            ctx.moveTo(w * 0.38, h * 0.68);
            ctx.lineTo(w * 0.62, h * 0.68);
            ctx.arc(cx, h * 0.68, w * 0.12, 0, Math.PI, false);
            ctx.fill();
            break;
        case "bellOff":
            ctx.beginPath();
            ctx.arc(cx, cy - h * 0.05, w * 0.28, Math.PI, 0, false);
            ctx.lineTo(w * 0.78, h * 0.62);
            ctx.lineTo(w * 0.22, h * 0.62);
            ctx.closePath();
            ctx.globalAlpha = 0.35;
            ctx.fill();
            ctx.globalAlpha = 1.0;
            ctx.beginPath();
            ctx.moveTo(w * 0.15, h * 0.15);
            ctx.lineTo(w * 0.85, h * 0.85);
            ctx.stroke();
            break;
        case "sun":
            ctx.beginPath();
            ctx.arc(cx, cy, w * 0.18, 0, 2 * Math.PI);
            ctx.fill();
            for (var i = 0; i < 8; i++) {
                var a = i * Math.PI / 4;
                var r1 = w * 0.28, r2 = w * 0.42;
                ctx.beginPath();
                ctx.moveTo(cx + r1 * Math.cos(a), cy + r1 * Math.sin(a));
                ctx.lineTo(cx + r2 * Math.cos(a), cy + r2 * Math.sin(a));
                ctx.stroke();
            }
            break;
        case "moon":
            ctx.beginPath();
            ctx.arc(cx + w * 0.06, cy, w * 0.3, 0, 2 * Math.PI);
            ctx.fill();
            ctx.globalCompositeOperation = "destination-out";
            ctx.beginPath();
            ctx.arc(cx + w * 0.18, cy - h * 0.08, w * 0.26, 0, 2 * Math.PI);
            ctx.fill();
            ctx.globalCompositeOperation = "source-over";
            break;
        case "info":
            ctx.beginPath();
            ctx.arc(cx, cy, w * 0.38, 0, 2 * Math.PI);
            ctx.stroke();
            ctx.beginPath();
            ctx.arc(cx, h * 0.32, w * 0.045, 0, 2 * Math.PI);
            ctx.fill();
            ctx.beginPath();
            ctx.moveTo(cx, h * 0.46);
            ctx.lineTo(cx, h * 0.72);
            ctx.stroke();
            break;
        case "play":
            ctx.beginPath();
            ctx.moveTo(w * 0.32, h * 0.22);
            ctx.lineTo(w * 0.32, h * 0.78);
            ctx.lineTo(w * 0.8, h * 0.5);
            ctx.closePath();
            ctx.fill();
            break;
        case "record":
            ctx.beginPath();
            ctx.arc(cx, cy, w * 0.3, 0, 2 * Math.PI);
            ctx.fill();
            break;
        case "chevron":
            ctx.beginPath();
            ctx.moveTo(w * 0.28, h * 0.36);
            ctx.lineTo(cx, h * 0.64);
            ctx.lineTo(w * 0.72, h * 0.36);
            ctx.stroke();
            break;
        case "warning":
            ctx.beginPath();
            ctx.moveTo(cx, h * 0.15);
            ctx.lineTo(w * 0.88, h * 0.82);
            ctx.lineTo(w * 0.12, h * 0.82);
            ctx.closePath();
            ctx.stroke();
            ctx.beginPath();
            ctx.moveTo(cx, h * 0.4);
            ctx.lineTo(cx, h * 0.62);
            ctx.stroke();
            ctx.beginPath();
            ctx.arc(cx, h * 0.72, w * 0.025, 0, 2 * Math.PI);
            ctx.fill();
            break;
        case "check":
            ctx.beginPath();
            ctx.moveTo(w * 0.2, h * 0.52);
            ctx.lineTo(w * 0.42, h * 0.74);
            ctx.lineTo(w * 0.82, h * 0.28);
            ctx.stroke();
            break;
        }
    }
}
