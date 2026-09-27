set (CMAKE_VS_GLOBALS UseMultiToolTask=true EnforceProcessCountAcrossBuilds=true)

# Чтение метаданных аддона из config.json (формат совпадает с официальным
# шаблоном GRAPHISOFT archicad-addon-cmake).
# Версия — только число, 1-3 компоненты, каждая 0-65535: из неё собирается
# FILEVERSION в VersionInfo.rc, поэтому префикс "v" недопустим.
set (GS_CONFIG_JSON_PATH "${CMAKE_SOURCE_DIR}/config.json" CACHE FILEPATH "")
mark_as_advanced (GS_CONFIG_JSON_PATH)

function (parse_version inValue outList)
    set (v1 0)
    set (v2 0)
    set (v3 0)
    unset (CMAKE_MATCH_COUNT)
    if (inValue MATCHES [[^([0-9]+)\.([0-9]+)\.([0-9]+)$]])
    elseif (inValue MATCHES [[^([0-9]+)\.([0-9]+)$]])
    elseif (inValue MATCHES [[^([0-9]+)$]])
    endif ()
    if (DEFINED CMAKE_MATCH_COUNT)
        foreach (i RANGE 1 "${CMAKE_MATCH_COUNT}")
            set ("v${i}" "${CMAKE_MATCH_${i}}")
            if ("${v${i}}" LESS "0" OR "${v${i}}" GREATER "65535")
                message (FATAL_ERROR "Component ${i} of version number '${inValue}' is outside the 0-65535 range.")
            endif ()
        endforeach ()
        set ("${outList}" "${v1};${v2};${v3}" PARENT_SCOPE)
    else ()
        unset ("${outList}" PARENT_SCOPE)
    endif ()
endfunction ()

function (ReadConfigJson)
    if (NOT EXISTS "${GS_CONFIG_JSON_PATH}")
        message (FATAL_ERROR "Config file not found: ${GS_CONFIG_JSON_PATH}")
    endif ()
    file (READ "${GS_CONFIG_JSON_PATH}" json)

    set (requiredMembers addOnName version description)
    set (returnAs addOnName addOnVersion addOnDescription)
    foreach (out members IN ZIP_LISTS returnAs requiredMembers)
        string (JSON "${out}" ERROR_VARIABLE error GET "${json}" ${members})
        if (error)
            message (FATAL_ERROR "Error getting required member (${members}): ${error}")
        endif ()
        set ("${out}" "${${out}}" PARENT_SCOPE)
    endforeach ()

    # Поля copyright читаем отдельными вызовами: вложенный путь в списке
    # ("copyright\;name") CMake разбирает как два отдельных элемента.
    string (JSON addOnCompanyName ERROR_VARIABLE error GET "${json}" copyright name)
    if (error)
        message (FATAL_ERROR "Error getting required member (copyright.name): ${error}")
    endif ()
    string (JSON addOnCopyrightYear ERROR_VARIABLE error GET "${json}" copyright year)
    if (error)
        message (FATAL_ERROR "Error getting required member (copyright.year): ${error}")
    endif ()

    # Год с плейсхолдером %Y разворачиваем в текущий год.
    if (addOnCopyrightYear MATCHES "%Y")
        string (TIMESTAMP currentYear "%Y")
        string (REPLACE "%Y" "${currentYear}" addOnCopyrightYear "${addOnCopyrightYear}")
    endif ()

    parse_version ("${addOnVersion}" addOnVersionParts)
    if (NOT DEFINED addOnVersionParts)
        message (FATAL_ERROR "'${addOnVersion}' does not follow the '123', '1.23' or '1.2.3' version format.")
    endif ()
    if (addOnVersionParts STREQUAL "0;0;0")
        message (WARNING "Add-on version is '0.0.0', a placeholder. Change it in 'config.json'.")
    endif ()
    list (JOIN addOnVersionParts . addOnVersion)

    set (AC_ADDON_FOR_DISTRIBUTION OFF CACHE BOOL "")
endfunction ()

function (SetGlobalCompilerDefinitions acVersion)

    if (WIN32)
        add_definitions (-DUNICODE -D_UNICODE -D_ITERATOR_DEBUG_LEVEL=0)
        set (CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL PARENT_SCOPE)
    else ()
        add_definitions (-Dmacintosh=1)
        if (${acVersion} GREATER_EQUAL 26)
            set (CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "" FORCE)
        else()
            set (CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "" FORCE)
        endif ()
    endif ()
    add_definitions (-DACExtension)

endfunction ()

function (SetCompilerOptions target acVersion)

    if (${acVersion} LESS 27)
        target_compile_features (${target} PUBLIC cxx_std_14)
    elseif (${acVersion} LESS 29)
        target_compile_features (${target} PUBLIC cxx_std_17)
    else ()
        target_compile_features (${target} PUBLIC cxx_std_20)
    endif ()
    # DEBUG/TESTING нужны и профильной конфигурации: без них DBprnt/DBtest
    # компилируются в пустоту и подтвердить загрузку сборки нечем.
    target_compile_options (${target} PUBLIC "$<$<OR:$<CONFIG:Debug>,$<CONFIG:ProfileDebug>>:-DDEBUG>")
    target_compile_options (${target} PUBLIC "$<$<OR:$<CONFIG:Debug>,$<CONFIG:ProfileDebug>>:-DTESTING>")
    if (WIN32)
        # В ProfileDebug нет флагов из CMAKE_CXX_FLAGS_DEBUG (нет /Od и /RTC1),
        # но и оптимизации по умолчанию тоже нет — задаём явно.
        # /Gy (function-level linking) и /Gw нужны, чтобы /PROFILE мог переставлять
        # функции и данные по частоте; /Zi — символы для привязки образцов к строкам.
        # /GL намеренно не задаётся: требует LTCG-метаданных во входных .lib DevKit,
        # при /WX предупреждения LTCG станут фатальными.
        target_compile_options (${target} PUBLIC
            "$<$<CONFIG:ProfileDebug>:/O2>"
            "$<$<CONFIG:ProfileDebug>:/Gy>"
            "$<$<CONFIG:ProfileDebug>:/Gw>"
            "$<$<CONFIG:ProfileDebug>:/Zi>"
            # /O2 поднимает ложное C4724 ("возможный остаток от деления на 0") в заголовке
            # DevKit GSRoot/HashSet.hpp:1235 (hashEntry.item % newHashListCount); заголовок
            # не наш, править нельзя, а /WX делает предупреждение ошибкой. Гасим только
            # в этой конфигурации, чтобы не ослаблять /WX для остальных сборок.
            "$<$<CONFIG:ProfileDebug>:/wd4724>"
        )
        target_compile_options (${target} PUBLIC /W3 /WX
            /Zc:wchar_t-
            /wd4499
            /EHsc
            /wd4003
            /wd5208
            /wd4996
            /bigobj
            -D_CRT_SECURE_NO_WARNINGS
        )
    else ()
        target_compile_options (${target} PUBLIC -Wall -Wextra -Werror
            -fvisibility=hidden
            -Wno-multichar
            -Wno-ctor-dtor-privacy
            -Wno-invalid-offsetof
            -Wno-ignored-qualifiers
            -Wno-reorder
            -Wno-overloaded-virtual
            -Wno-unused-parameter
            -Wno-unused-value
            -Wno-unused-private-field
            -Wno-unused-but-set-variable
            -Wno-unused-variable
            -Wno-unused-function
            -Wno-deprecated
            -Wno-unknown-pragmas
            -Wno-missing-braces
            -Wno-missing-field-initializers
            -Wno-non-c-typedef-for-linkage
            -Wno-uninitialized-const-reference
            -Wno-shorten-64-to-32
            -Wno-sign-compare
            -Wno-switch
            -Wno-missing-template-arg-list-after-template-kw
            -Wno-ambiguous-operators
            -Wno-unknown-warning-option
        )
        if (${acVersion} LESS_EQUAL "24")
            target_compile_options (${target} PUBLIC -Wno-non-c-typedef-for-linkage)
        endif ()
    endif ()
    add_definitions (-DAC_${acVersion})
endfunction ()

function (DetectACVersion devKitDir acVersion)

    message (STATUS "AI_CMAKE_STATUS [DETECT_AC_VERSION] devkit='${devKitDir}'")
    set (ACAPIncFileLocation ${devKitDir}/Inc/ACAPinc.h)
    if (EXISTS ${ACAPIncFileLocation})
        file (READ ${ACAPIncFileLocation} ACAPIncContent)
        string (REGEX MATCHALL "#define[ \t]+ServerMainVers_([0-9][0-9])" VersionList ${ACAPIncContent})
        set (${acVersion} ${CMAKE_MATCH_1} PARENT_SCOPE)
        message (STATUS "AI_CMAKE_STATUS [DETECT_AC_VERSION_OK] version='${CMAKE_MATCH_1}' header='${ACAPIncFileLocation}'")
    else ()
        message (STATUS "AI_CMAKE_STATUS [DETECT_AC_VERSION_FAILED] reason='ACAPinc.h not found' header='${ACAPIncFileLocation}'")
        message (FATAL_ERROR "Failed to detect Archicad version, please check the value of the AC_API_DEVKIT_DIR variable.")
    endif ()

endfunction ()

function (LinkGSLibrariesToProject acVersion devKitDir addOnName)

    message (STATUS "AI_CMAKE_STATUS [LINK_GS_LIBRARIES] addon='${addOnName}' ac_version='${acVersion}' devkit='${devKitDir}'")
    if (WIN32)
        set_target_properties(${addOnName} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$(ProjectDir)"
        VS_DEBUGGER_COMMAND "$ENV{ProgramFiles}/GRAPHISOFT/ARCHICAD ${acVersion}/ARCHICAD.exe"
        VS_DEBUGGER_COMMAND_ARGUMENTS "$(ProjectDir)test_${acVersion}.pln -forceaccessdialog -bringToFront -DISABLERECOVERYDIALOG"
        )
        if (${acVersion} LESS 27)
            target_link_libraries (${addOnName}
                "${devKitDir}/Lib/Win/ACAP_STAT.lib"
            )
        else ()
            target_link_libraries (${addOnName}
                "${devKitDir}/Lib/ACAP_STAT.lib"
            )
        endif ()
    else ()
        find_library (CocoaFramework Cocoa)
        if (${acVersion} LESS 27)
            target_link_libraries (${addOnName}
                "${devKitDir}/Lib/Mactel/libACAP_STAT.a"
                ${CocoaFramework}
            )
        else ()
            target_link_libraries (${addOnName}
                "${devKitDir}/Lib/libACAP_STAT.a"
                ${CocoaFramework}
            )
        endif ()
    endif ()

    file (GLOB ModuleFolders ${devKitDir}/Modules/*)
    target_include_directories (${addOnName} PUBLIC ${ModuleFolders})
    if (WIN32)
        file (GLOB LibFilesInFolder ${devKitDir}/Modules/*/*/*.lib)
        target_link_libraries (${addOnName} ${LibFilesInFolder})
    else ()
        file (GLOB LibFilesInFolder
            ${devKitDir}/Frameworks/*.framework
            ${devKitDir}/Frameworks/*.dylib
        )
        target_link_libraries (${addOnName} ${LibFilesInFolder})
    endif ()

endfunction ()

function (GenerateAddOnProject acVersion devKitDir addOnName addOnSourcesFolder addOnResourcesFolder addOnLanguage)

    message (STATUS "AI_CMAKE_STATUS [GENERATE_ADDON_PROJECT] addon='${addOnName}' ac_version='${acVersion}' language='${addOnLanguage}' sources='${addOnSourcesFolder}' resources='${addOnResourcesFolder}'")
    find_package (Python COMPONENTS Interpreter)

    set (ResourceObjectsDir ${CMAKE_BINARY_DIR}/ResourceObjects)
    set (ResourceStampFile "${ResourceObjectsDir}/AddOnResources.stamp")

    file (GLOB AddOnImageFiles CONFIGURE_DEPENDS
        ${addOnResourcesFolder}/RFIX/Images/*.svg
    )
    if (WIN32)
        file (GLOB AddOnResourceFiles CONFIGURE_DEPENDS
            ${addOnResourcesFolder}/R${addOnLanguage}/*.grc
            ${addOnResourcesFolder}/RFIX/*.grc
            ${addOnResourcesFolder}/RFIX.win/*.rc2
            ${addOnResourcesFolder}/RFIX/HTML/*.html
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/*.py
        )
    else ()
        file (GLOB AddOnResourceFiles CONFIGURE_DEPENDS
            ${addOnResourcesFolder}/R${addOnLanguage}/*.grc
            ${addOnResourcesFolder}/RFIX/*.grc
            ${addOnResourcesFolder}/RFIX.mac/*.plist
            ${addOnResourcesFolder}/RFIX/HTML/*.html
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/*.py
        )
    endif ()

    get_filename_component (AddOnSourcesFolderAbsolute "${CMAKE_CURRENT_LIST_DIR}/${addOnSourcesFolder}" ABSOLUTE)
    get_filename_component (AddOnResourcesFolderAbsolute "${CMAKE_CURRENT_LIST_DIR}/${addOnResourcesFolder}" ABSOLUTE)
    if (WIN32)
        add_custom_command (
            OUTPUT ${ResourceStampFile}
            DEPENDS ${AddOnResourceFiles} ${AddOnImageFiles}
            COMMENT "AI_CMAKE_STATUS [COMPILE_RESOURCES] platform='WIN' addon='${addOnName}' language='${addOnLanguage}'"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${ResourceObjectsDir}"
            COMMAND ${Python_EXECUTABLE} "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/CompileResources.py" "${addOnLanguage}" "${devKitDir}" "${AddOnSourcesFolderAbsolute}" "${AddOnResourcesFolderAbsolute}" "${ResourceObjectsDir}" "${ResourceObjectsDir}/${addOnName}.res"
            COMMAND ${CMAKE_COMMAND} -E touch ${ResourceStampFile}
        )
    else ()
        add_custom_command (
            OUTPUT ${ResourceStampFile}
            DEPENDS ${AddOnResourceFiles} ${AddOnImageFiles}
            COMMENT "AI_CMAKE_STATUS [COMPILE_RESOURCES] platform='MAC' addon='${addOnName}' language='${addOnLanguage}'"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${ResourceObjectsDir}"
            COMMAND ${Python_EXECUTABLE} "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/CompileResources.py" "${addOnLanguage}" "${devKitDir}" "${AddOnSourcesFolderAbsolute}" "${AddOnResourcesFolderAbsolute}" "${ResourceObjectsDir}" "${CMAKE_BINARY_DIR}/$<CONFIG>/${addOnName}.bundle/Contents/Resources"
            COMMAND ${CMAKE_COMMAND} -E copy "${devKitDir}/Inc/PkgInfo" "${CMAKE_BINARY_DIR}/$<CONFIG>/${addOnName}.bundle/Contents/PkgInfo"
            COMMAND ${CMAKE_COMMAND} -E touch ${ResourceStampFile}
        )
    endif ()

    file (GLOB_RECURSE AddOnHeaderFiles CONFIGURE_DEPENDS
        ${addOnSourcesFolder}/*.h
    )
    file (GLOB_RECURSE AddOnSourceFiles CONFIGURE_DEPENDS
        ${addOnSourcesFolder}/*.cpp
    )
    set (
        AddOnFiles
        ${AddOnHeaderFiles}
        ${AddOnSourceFiles}
        ${AddOnImageFiles}
        ${AddOnResourceFiles}
        ${ResourceStampFile}
    )
    
    source_group (TREE ${CMAKE_CURRENT_LIST_DIR}/${addOnSourcesFolder}
        PREFIX "Sources"
        FILES ${AddOnHeaderFiles} ${AddOnSourceFiles}
    )
    source_group ("Images" FILES ${AddOnImageFiles})
    source_group ("Resources" FILES ${AddOnResourceFiles})
    if (WIN32)
        add_library (${addOnName} SHARED ${AddOnFiles})
    else ()
        add_library (${addOnName} MODULE ${AddOnFiles})
    endif ()

    # Версия аддона: из config.json (число, без "v"). CMake-переменная нужна для
    # configure_file ниже, compile definition — для исходников C++.
    if (NOT addOnVersion)
        message (FATAL_ERROR "addOnVersion is empty. Call ReadConfigJson () before GenerateAddOnProject ().")
    endif ()
    set (ADDON_VERSION ${addOnVersion})
    set (ADDON_NAME ${addOnName})
    string(TIMESTAMP addonsubversion "%Y-%m-%d-%H")
    set(ADDON_SUBVERSION ${addonsubversion})
    
    configure_file(
                "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/AddOn.grc.in"
                "${CMAKE_CURRENT_LIST_DIR}/${addOnResourcesFolder}/RINT/AddOn.grc"
                @ONLY
            )

    set_target_properties (${addOnName} PROPERTIES OUTPUT_NAME ${addOnName})
    if (WIN32)
        set_target_properties (${addOnName} PROPERTIES SUFFIX ".apx")
        set_target_properties (${addOnName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_$<CONFIG> "${CMAKE_BINARY_DIR}/$<CONFIG>")
        # Профильная сборка кладёт .apx в ту же папку Debug: Add-On Manager грузит
        # .../25/Debug/SomeStuff.apx, поэтому артефакт подхватывается без копирования.
        # Перезапись Debug-сборки намеренная — профилирование и обычная отладка
        # не сосуществуют; суффикс к имени не добавляется, иначе модуль не загрузится.
        # Имя свойства — с конфигурацией в ВЕРХНЕМ регистре: CMake ищет
        # RUNTIME_OUTPUT_DIRECTORY_<CONFIG-UPPER>, поэтому ProfileDebug не подходит.
        set_target_properties (${addOnName} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_PROFILEDEBUG "${CMAKE_BINARY_DIR}/Debug")
        # /PROFILE — опция линковщика (не компилятора): даёт образ с привязкой символов
        # для VSInstr и семплера. /DEBUG обязателен: в ProfileDebug нет /Zi-/DEBUG из
        # CMAKE_*_FLAGS_DEBUG, поэтому CMake ставит GenerateDebugInformation=false и
        # PDB не создаётся — символы компиляции остаются неиспользованными.
        # /INCREMENTAL:NO — при /PROFILE инкрементальная линковка неприменима (LNK4075).
        target_link_options (${addOnName} PUBLIC
            "$<$<CONFIG:ProfileDebug>:/PROFILE>"
            "$<$<CONFIG:ProfileDebug>:/DEBUG>"
            "$<$<CONFIG:ProfileDebug>:/INCREMENTAL:NO>"
        )
        target_link_options (${addOnName} PUBLIC "${ResourceObjectsDir}/${addOnName}.res")
        target_link_options (${addOnName} PUBLIC /export:GetExportedFuncAddrs,@1 /export:SetImportedFuncAddrs,@2)
    else ()
        file(READ "${devKitDir}/Frameworks/GSRoot.framework/Versions/A/Resources/Info.plist" plist_content NEWLINE_CONSUME)
        string (REGEX MATCH "GSBuildNum[^0-9]+([0-9]+)" unused "${plist_content}")
        set (gsBuildNum "${CMAKE_MATCH_1}")
        string (REGEX MATCH "LSMinimumSystemVersion[^0-9]+([0-9.]+)" unused "${plist_content}")
        set (lsMinimumSystemVersion "${CMAKE_MATCH_1}")

        set(MACOSX_BUNDLE_EXECUTABLE_NAME ${addOnName})
        set(MACOSX_BUNDLE_INFO_STRING ${addOnName})
        set(MACOSX_BUNDLE_LONG_VERSION_STRING ${copyright})
        set(MACOSX_BUNDLE_BUNDLE_NAME ${addOnName})
        set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${acVersion}.0.0.${gsBuildNum})
        set(MACOSX_BUNDLE_BUNDLE_VERSION ${acVersion}.0.0.${gsBuildNum})
        set(MINIMUM_SYSTEM_VERSION "${lsMinimumSystemVersion}")
    
        configure_file(
                "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/AddOn.plist.in"
                "${CMAKE_CURRENT_LIST_DIR}/${addOnResourcesFolder}/RFIX.mac/Info.plist"
                @ONLY
            )

        set_target_properties(${addOnName} PROPERTIES
            BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/${addOnResourcesFolder}/RFIX.mac/Info.plist"

            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER com.kuvbur.${addOnNameIdentifier}
            XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET ${lsMinimumSystemVersion}

            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
        )
    endif ()

    target_precompile_headers(
        "${addOnName}" PRIVATE
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/AddOn.hpp"
    )
    
    target_include_directories (${addOnName} PUBLIC
        ${addOnSourcesFolder}
        ${devKitDir}/Inc
    )

    LinkGSLibrariesToProject (${acVersion} ${devKitDir} ${addOnName})

    target_compile_definitions (${addOnName} PRIVATE
        "ADDON_VERSION=\"${ADDON_VERSION}\""
        "ADDON_NAME=\"${ADDON_NAME}\""
        "ADDON_LANGUAGE=\"${addOnLanguage}\""
    )

    set_source_files_properties (${AddOnSourceFiles} PROPERTIES LANGUAGE CXX)
    SetCompilerOptions (${addOnName} ${acVersion})
    message (STATUS "AI_CMAKE_STATUS [GENERATE_ADDON_PROJECT_OK] addon='${addOnName}' ac_version='${acVersion}' language='${addOnLanguage}'")

endfunction ()
