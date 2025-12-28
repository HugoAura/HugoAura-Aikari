import QtQuick
import QtQuick.Shapes

Shape {
    id: diamondShapeRoot
    property real customScale: 1
    property alias strokeColor: diamondShapePath.strokeColor
    property alias strokeWidth: diamondShapePath.strokeWidth
    property alias fillColor: diamondShapePath.fillColor

    width: 14 * customScale
    height: 30 * customScale

    preferredRendererType: Shape.GeometryRenderer
    layer.enabled: true
    layer.smooth: true
    layer.samples: 4

    ShapePath {
        id: diamondShapePath

        joinStyle: ShapePath.MiterJoin
        capStyle: ShapePath.FlatCap

        startX: 7 * customScale
        startY: 2 * customScale

        PathLine {
            x: 12 * customScale
            y: 15 * customScale
        }
        PathLine {
            x: 7 * customScale
            y: 28 * customScale
        }
        PathLine {
            x: 2 * customScale
            y: 15 * customScale
        }
        PathLine {
            x: 7 * customScale
            y: 2 * customScale
        }
    }
}
