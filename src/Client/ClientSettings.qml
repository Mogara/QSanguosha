import Cardirector.Client 1.0

ClientSettings {
    id: config

    property string backgroundImage: config.value("backgroundImage", "image://background/bg")

    onBackgroundImageChanged: config.setValue("backgroundImage", backgroundImage)
}
