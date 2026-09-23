pragma Singleton
import QtQuick

// Design tokens for the Operator Assistant UI.
// Values are carried over from the reference design system (dark industrial
// telemetry console: single accent, status-driven color coding, large
// display numerals, uppercase tracked labels) - fitting for a kiosk-style
// in-cab touchscreen. Product-specific names from that source were dropped;
// only the token values and component patterns were kept.
QtObject {
    // ---- Surfaces ------------------------------------------------------
    readonly property color surfaceBase: "#181818"     // App/page background
    readonly property color surfaceCard: "#2c2c2c"     // Cards, panels
    readonly property color surfaceAlt: "#292828"      // Reserved secondary surface
    readonly property color controlBg: "#2a2a2a"       // Dropdowns, inputs

    readonly property color borderPanel: "#4c4a4a"     // Panel outline (intentionally low-contrast)
    readonly property color borderControl: "#3d3d3d"   // Control outline

    // ---- Ink -------------------------------------------------------------
    readonly property color ink: "#ffffff"
    readonly property color inkSecondary: "#c0c0c0"
    readonly property color inkTertiary: "#a1a1a1"

    // ---- Accent & status --------------------------------------------------
    readonly property color accent: "#00bbf9"
    readonly property color statusNormal: accent
    readonly property color statusWarning: "#ffaa05"
    readonly property color statusCritical: "#ff002a"   // meets contrast at large sizes/icons only

    readonly property color consoleText: "#00ff41"      // log console monospace text
    readonly property color scrim: "#aa000000"
    readonly property color shadowColor: "#80000000"

    readonly property color actionSecondary: "#2196f3"        // non-primary action fill
    readonly property color actionSecondaryPressed: "#1565c0"

    // ---- Typography --------------------------------------------------------
    readonly property string fontDisplay: "Verdana"     // numerals only
    readonly property string fontSans: "Ubuntu Sans"    // labels, headings, buttons
    readonly property string fontMono: "Consolas"       // logs / debug readouts

    // Display (big numerals)
    readonly property int displayXl: 72        // hero live metric
    readonly property int displayL: 44         // secondary max/min-style numeral
    readonly property int displayUnit: 28
    readonly property int displayUnitSm: 20

    // Headings
    readonly property int headingWordmark: 26
    readonly property int headingSection: 20
    readonly property real headingSectionTracking: 3

    // Body
    readonly property int bodySize: 15
    readonly property int bodyStrong: 17
    readonly property int captionSize: 13
    readonly property int popupTitle: 18
    readonly property int popupBody: 14
    readonly property int actionLabelSize: 13

    // Mono
    readonly property int consoleSize: 13
    readonly property int hudSize: 12

    // ---- Spacing -----------------------------------------------------------
    readonly property int space5: 5
    readonly property int space10: 10
    readonly property int space15: 15
    readonly property int space20: 20
    readonly property int space30: 30
    readonly property int space50: 50

    // ---- Radius ------------------------------------------------------------
    readonly property int radiusSm: 5
    readonly property int radiusMd: 8
    readonly property int radiusLg: 12
    readonly property int radiusXl: 20
    readonly property int radiusPill: 999

    // ---- Shadows (offsetX, offsetY, blur, color) ----------------------------
    // QtQuick has no native box-shadow; components approximate these with a
    // MultiEffect/dropShadow using shadowColor at the given offset/blur.
    readonly property var shadowCard: ({ dx: 0, dy: 3, blur: 10 })
    readonly property var shadowBadge: ({ dx: 0, dy: 3, blur: 6 })
    readonly property var shadowModal: ({ dx: 0, dy: 10, blur: 20 })

    // ---- Status helper ------------------------------------------------------
    function statusColor(level) {
        if (level === "warning") return statusWarning;
        if (level === "critical") return statusCritical;
        return statusNormal;
    }
}
