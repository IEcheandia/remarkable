qt5_add_translation(qmFiles
    weldmaster.de.ts
    weldmaster.en.ts
    weldmaster.zh.ts
    weldmaster.fr.ts
    ../../App_Grapheditor/ts/weldmaster-grapheditor_de.ts
    ../../App_Grapheditor/ts/weldmaster-grapheditor_en.ts
    ../../App_Grapheditor/ts/weldmaster-grapheditor_fr.ts
    ../../App_Grapheditor/ts/weldmaster-grapheditor_zh.ts
    ../../App_WobbelFigureEditor/ts/weldmaster-figureditor_de.ts
    ../../App_WobbelFigureEditor/ts/weldmaster-figureditor_en.ts
    ../../App_WobbelFigureEditor/ts/weldmaster-figureditor_fr.ts
    ../../App_WobbelFigureEditor/ts/weldmaster-figureditor_zh.ts
    ../../Mod_Storage/ts/weldmaster-storage_de.ts
    ../../Mod_Storage/ts/weldmaster-storage_en.ts
    ../../Mod_Storage/ts/weldmaster-storage_fr.ts
    ../../Mod_Storage/ts/weldmaster-storage_zh.ts
)
add_custom_target(QMS ALL DEPENDS ${qmFiles})
install(
    FILES
        ${qmFiles}
    DESTINATION
        ${WM_INSTALL_DIR}/languages
)

