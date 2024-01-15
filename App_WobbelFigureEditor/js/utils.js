// Checks the return value of FileModel.loadJsonFromFile(), notifies if it is an error and returns a boolean if loading was successful
function checkLoadingFeedback(returnValue) {
    if (returnValue === FileModel.EmptyFile)
    {
        Notifications.NotificationSystem.error(qsTr("File is empty!"));
        return false
    }
    if (returnValue === FileModel.CorruptedFile) {
        Notifications.NotificationSystem.error(
                    qsTr("File is corrupted!"))
        return false
    }
    if (returnValue !== FileModel.NoErrors) {
        Notifications.NotificationSystem.error(qsTr("Unexpected LoadingFeedback: %1").arg(returnValue))
        return false
    }

    return true
}


