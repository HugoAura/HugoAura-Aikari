import QtQuick
import QtQuick.Shapes

Shape {
    id: clippedRectangleMaskRoot
    // required baseProperty width
    // required baseProperty height

    property int clipTriangleSize: 5

    // preferredRendererType: Shape.GeometryRenderer
    layer.enabled: true
    layer.smooth: true
    layer.samples: 4

    ShapePath {
        id: clippedRectangleMaskPath

        strokeWidth: 0
        fillColor: "white"

        joinStyle: ShapePath.MiterJoin
        capStyle: ShapePath.FlatCap

        startX: clippedRectangleMaskRoot.clipTriangleSize
        startY: 0

        PathLine {
            x: clippedRectangleMaskRoot.width
            y: 0
        }

        PathLine {
            x: clippedRectangleMaskRoot.width
            y: clippedRectangleMaskRoot.height -
                clippedRectangleMaskRoot.clipTriangleSize
        }

        PathLine {
            x: clippedRectangleMaskRoot.width -
                clippedRectangleMaskRoot.clipTriangleSize
            y: clippedRectangleMaskRoot.height
        }

        PathLine {
            x: 0
            y: clippedRectangleMaskRoot.height
        }

        PathLine {
            x: 0
            y: clippedRectangleMaskRoot.clipTriangleSize
        }

        PathLine {
            x: clippedRectangleMaskRoot.clipTriangleSize
            y: 0
        }
    }
}
