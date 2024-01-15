import QtQuick 2.8
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtWebEngine 1.8

ColumnLayout {

    WebEngineView {
        id: webView
        url: "https://localhost:4680/"
        Layout.fillWidth: true
        Layout.fillHeight: true
        onCertificateError: error.ignoreCertificateError()
    }
    
}

