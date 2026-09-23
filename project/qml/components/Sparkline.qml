import QtQuick

// Small trend polyline, in the spirit of the reference design's raw SVG
// waveform on each metric card.
Canvas {
    id: root
    property var points: []     // array of numbers
    property color lineColor: "#00bbf9"
    height: 32

    onPointsChanged: requestPaint()
    onLineColorChanged: requestPaint()
    onWidthChanged: requestPaint()

    onPaint: {
        var ctx = getContext("2d");
        ctx.reset();
        if (!points || points.length < 2) return;

        var minV = Math.min.apply(Math, points);
        var maxV = Math.max.apply(Math, points);
        var span = (maxV - minV) < 1e-6 ? 1.0 : (maxV - minV);
        var stepX = width / (points.length - 1);
        var pad = height * 0.15;

        ctx.strokeStyle = lineColor;
        ctx.lineWidth = 2.5;
        ctx.lineCap = "round";
        ctx.lineJoin = "round";
        ctx.beginPath();
        for (var i = 0; i < points.length; i++) {
            var x = i * stepX;
            var norm = (points[i] - minV) / span;
            var y = height - pad - norm * (height - 2 * pad);
            if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
        }
        ctx.stroke();
    }
}
