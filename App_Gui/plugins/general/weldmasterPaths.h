#pragma once

#include <QObject>

namespace precitec
{
namespace gui
{

/**
 * Singleton class providing access to all the known paths in a Weldmaster installation
 **/
class WeldmasterPaths : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString videoBaseDir READ videoBaseDir CONSTANT)
    /**
     * Directory containing the results per product instance
     **/
    Q_PROPERTY(QString resultsBaseDir READ resultsBaseDir CONSTANT)
    /**
     * Directory containing the xml definitions of the graphs (user installed)
     **/
    Q_PROPERTY(QString graphDir READ graphDir CONSTANT)
    /**
     * Directory containing the xml definitions of the graphs (system installed)
     **/
    Q_PROPERTY(QString systemGraphDir READ systemGraphDir CONSTANT)
    /**
     * Directory containing the xml definitions of the sub graphs in the systemGraphDir (system installed)
     **/
    Q_PROPERTY(QString subGraphDir READ subGraphDir CONSTANT)
    /**
     * Directory containing the xml definitions of the sub graphs (user installed)
     **/
    Q_PROPERTY(QString userSubGraphDir READ userSubGraphDir CONSTANT)

    Q_PROPERTY(QString subGraphInputDir READ subGraphInputDir CONSTANT)
    Q_PROPERTY(QString subGraphROIDir READ subGraphROIDir CONSTANT)
    Q_PROPERTY(QString subGraphPreDir READ subGraphPreDir CONSTANT)
    Q_PROPERTY(QString subGraphProcessDir READ subGraphProcessDir CONSTANT)
    Q_PROPERTY(QString subGraphPostDir READ subGraphPostDir CONSTANT)
    Q_PROPERTY(QString subGraphOutputDir READ subGraphOutputDir CONSTANT)
    Q_PROPERTY(QString subGraphS6KSpecialS1Dir READ subGraphS6KSpecialS1Dir CONSTANT)
    Q_PROPERTY(QString subGraphS6KSpecialS2Dir READ subGraphS6KSpecialS2Dir CONSTANT)
    /**
     * Directory containing assembly images
     **/
    Q_PROPERTY(QString assemblyImagesDir READ assemblyImagesDir CONSTANT)
    /**
     * The configuration directory.
     **/
    Q_PROPERTY(QString configurationDir READ configurationDir CONSTANT)
    /**
     * Directory containg the products
     **/
    Q_PROPERTY(QString productDir READ productDir CONSTANT)
    /**
     * The directory where updates are archived
     **/
    Q_PROPERTY(QString updateArchiveDir READ updateArchiveDir CONSTANT)
    /**
     * The log directory.
     **/
    Q_PROPERTY(QString logfilesDir READ logfilesDir CONSTANT)
    /**
     * The directory where online help pdfs are stored
     **/
    Q_PROPERTY(QString pdfFilesDir READ pdfFilesDir CONSTANT)
    /**
     * Directory containing saved reference curves
     **/
    Q_PROPERTY(QString referenceCruveDir READ referenceCruveDir CONSTANT)
    /**
     * Directory containing local backups of hardware configuration
     **/
    Q_PROPERTY(QString hardwareConfigurationBackupDir READ hardwareConfigurationBackupDir CONSTANT)
    /**
     * Directory containing saved laser control configurations
     **/
    Q_PROPERTY(QString laserControlPresetDir READ laserControlPresetDir CONSTANT)
    /**
     * Directory containing configuration (certificates) for openVpn
     **/
    Q_PROPERTY(QString openVpnConfigDir READ openVpnConfigDir CONSTANT)
    /**
     * Directory containing configuration templates
     **/
    Q_PROPERTY(QString configurationTemplatesDir READ configurationTemplatesDir CONSTANT)

    Q_PROPERTY(QString languageDir READ languageDir CONSTANT)

    Q_PROPERTY(QString filterLibDir READ filterLibDir CONSTANT)

    Q_PROPERTY(QString filterPictureDir READ filterPictureDir CONSTANT)
    /**
     * Directory containing scan field images
     **/
    Q_PROPERTY(QString scanFieldDir READ scanFieldDir CONSTANT)
    /**
     * Directory containing the camera reference images
     **/
    Q_PROPERTY(QString referenceImageDir READ referenceImageDir CONSTANT)
    /**
     * Directory containg the debug data for the scanmaster calibration
     **/
    Q_PROPERTY(QString scanfieldDataDir READ scanfieldDataDir CONSTANT)
    /**
     * Directory containing system graph macros
     **/
    Q_PROPERTY(QString systemMacroDir READ systemMacroDir CONSTANT)
    /**
     * Directory containing user graph macros
     **/
    Q_PROPERTY(QString userMacroDir READ userMacroDir CONSTANT)

    /**
     * Base directory of weldmaster installation
     **/
    Q_PROPERTY(QString baseDirectory READ baseDirectory CONSTANT)

public:
    ~WeldmasterPaths() override;

    static WeldmasterPaths *instance();

    QString videoBaseDir() const;

    QString resultsBaseDir() const;

    QString graphDir() const;

    QString systemGraphDir() const;

    QString subGraphDir() const;

    QString userSubGraphDir() const;

    QString subGraphInputDir() const;
    QString subGraphROIDir() const;
    QString subGraphPreDir() const;
    QString subGraphProcessDir() const;
    QString subGraphPostDir() const;
    QString subGraphOutputDir() const;
    QString subGraphS6KSpecialS1Dir() const;
    QString subGraphS6KSpecialS2Dir() const;

    QString assemblyImagesDir() const;

    QString configurationDir() const;

    QString productDir() const;

    QString logfilesDir() const;

    QString updateArchiveDir() const;

    QString pdfFilesDir() const;

    QString hardwareConfigurationBackupDir() const;

    QString referenceCruveDir() const;

    QString laserControlPresetDir() const;

    QString openVpnConfigDir() const;

    QString configurationTemplatesDir() const;

    QString scanFieldDir() const;

    QString languageDir() const;

    QString baseDirectory() const;

    QString filterLibDir() const;

    QString filterPictureDir() const;

    QString referenceImageDir() const;

    QString scanfieldDataDir() const;

    QString systemMacroDir() const;

    QString userMacroDir() const;

private:
    explicit WeldmasterPaths();
};

}
}
