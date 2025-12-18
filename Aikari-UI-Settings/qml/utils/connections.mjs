export function createConnection(target, signalName, callback) {
    try {
        target[signalName].connect(callback);
        return {
            success: true,
            destroy: function () {
                try {
                    target[signalName].disconnect(callback);
                } catch (e) {
                    console.error("Error disconnecting connection. | Detail:", e);
                }
            }
        };
    } catch (e) {
        return {
            success: false,
        };
    }
}
