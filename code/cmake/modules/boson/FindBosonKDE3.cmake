# AB: this file adds some extra features to the default FindKDE3.cmake file of
#     official cmake.

# AB: atm nothing necessary (or has been added to FindKDE3.cmake directly)

#find_package(KDE3)
#find_package(KDE3)
find_package(ECM REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(ECMAddAppIcon)
include(ECMGenerateHeaders)
include(ECMInstallIcons)
include(ECMMarkNonGuiExecutable)
include(ECMOptionalAddSubdirectory)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
#include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)

find_package(KF5
    REQUIRED
    CoreAddons
    )

