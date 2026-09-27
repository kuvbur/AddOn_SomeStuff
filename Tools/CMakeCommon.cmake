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

    # Без PARENT_SCOPE значения остаются локальными внутри функции и во внешнем
    # scope не видны: VERSIONINFO-ресурс собирался с пустыми CompanyName и
    # обрезанным LegalCopyright.
    set (addOnCompanyName "${addOnCompanyName}" PARENT_SCOPE)
    set (addOnCopyrightYear "${addOnCopyrightYear}" PARENT_SCOPE)

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

function (verify_api_devkit_folder devKitPath)
    if (NOT EXISTS "${devKitPath}")
        message (FATAL_ERROR "The supplied API DevKit path ${devKitPath} does not exist")
    endif ()

    cmake_path (GET devKitPath FILENAME currentFolderName)
    if (NOT currentFolderName STREQUAL "Support")
        message (FATAL_ERROR "The supplied API DevKit path should point to the /Support subfolder of the API DevKit. Actual path: ${devKitPath}")
    endif ()

    if (NOT EXISTS "${devKitPath}/Lib")
        message (FATAL_ERROR "${devKitPath}/Lib does not exist")
    endif ()

    if (NOT EXISTS "${devKitPath}/Modules")
        message (FATAL_ERROR "${devKitPath}/Modules does not exist")
    endif ()

    if (APPLE AND NOT EXISTS "${devKitPath}/Frameworks")
        message (FATAL_ERROR "${devKitPath}/Frameworks does not exist")
    endif ()
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
            -fno-constant-cfstrings
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

    # SYSTEM: предупреждения из заголовков DevKit не наши — при /WX они стали бы
    # фатальными, и на каждый такой заголовок пришлось бы ставить ручной /wd.
    file (GLOB ModuleFolders ${devKitDir}/Modules/*)
    target_include_directories (${addOnName} SYSTEM PUBLIC ${ModuleFolders})
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

function (generate_add_on_version_info target acVersion outSemver)

    parse_version ("${addOnVersion}" vers)
    if (NOT DEFINED vers)
        message (FATAL_ERROR "'${addOnVersion}' does not follow the '123' or '1.23' or '1.2.3' version format.")
    endif ()
    if (vers STREQUAL "0;0;0")
        message (WARNING "Addon version is '0.0.0', which is a placeholder version. Please change it in 'config.json'.")
    endif ()

    list (JOIN vers . version)
    string (TIMESTAMP copyright "Copyright © ${addOnCompanyName}, ${addOnCopyrightYear}")

    if (WIN32)
        # gsBuildNum на Windows всегда 0: единственный источник — Info.plist
        # фреймворка GSRoot, которого на Windows нет (FIXME(HVA) в upstream).
        set (gsBuildNum 0)
        list (APPEND vers "${gsBuildNum}")
        list (JOIN vers , versionComma)

        # В строке ресурса кавычки и обратные слэши недопустимы. Подмена локальна
        # для функции — вызывающий scope не затрагивается.
        string (REGEX REPLACE [[(\\|")]] [[\\\1]] addOnDescription "${addOnDescription}")

        if (autoupdate STREQUAL "1")
            set (autoupdate "\n            VALUE \"Autoupdate\", \"1\"")
        else ()
            set (autoupdate "")
        endif ()

        # Translation для блока StringFileInfo. TODO: таблица константная, потому
        # что config.json объявляет только "INT" (0x0409 en-US, 0x04b0 Unicode).
        # При добавлении языков таблицу надо брать из GSLocalization.h
        # (WIN_LANGCHARSET_STR), как это делает upstream через
        # LocalizationMappingTable.py + AC_WIN_LANGCHARSET из BuildAddOn.py.
        set (winLangCharset "040904B0")
        set (winLanguageId 0x0409)
        set (winCharsetId 0x04B0)

        # STRS 18000: длина кода языка с завершающим нулём. Константа 4L из
        # upstream верна только для трёхбуквенных кодов, поэтому считаем.
        string (LENGTH "${addOnLanguage}" addOnLanguageLength)
        math (EXPR addOnLanguageLength "${addOnLanguageLength} + 1")

        foreach (res IN ITEMS VersionInfo AddOn)
            set (out "${CMAKE_CURRENT_BINARY_DIR}/${target}-${res}.rc")
            configure_file ("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/${res}.rc.in" "${out}" @ONLY)
            target_sources ("${target}" PRIVATE "${out}")
        endforeach ()
    endif ()

    set ("${outSemver}" "${version}" PARENT_SCOPE)
endfunction ()

function (GenerateAddOnProject acVersion devKitDir addOnName addOnSourcesFolder addOnResourcesFolder addOnLanguage)

    message (STATUS "AI_CMAKE_STATUS [GENERATE_ADDON_PROJECT] addon='${addOnName}' ac_version='${acVersion}' language='${addOnLanguage}' sources='${addOnSourcesFolder}' resources='${addOnResourcesFolder}'")
    # REQUIRED: без него конфигурация молча проходит с пустым Python3_EXECUTABLE,
    # и падение приходит позже — уже при запуске CompileResources.py, с
    # невнятным сообщением. Граница 3.8 — минимум текущего CI (Win 3.8, Mac 3.10).
    find_package (Python3 3.8 REQUIRED COMPONENTS Interpreter)
    message (STATUS "Using Python3 interpreter: ${Python3_EXECUTABLE}")

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
            COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/CompileResources.py" "${addOnLanguage}" "${devKitDir}" "${AddOnSourcesFolderAbsolute}" "${AddOnResourcesFolderAbsolute}" "${ResourceObjectsDir}" "${ResourceObjectsDir}/${addOnName}.res"
            COMMAND ${CMAKE_COMMAND} -E touch ${ResourceStampFile}
        )
    else ()
        add_custom_command (
            OUTPUT ${ResourceStampFile}
            DEPENDS ${AddOnResourceFiles} ${AddOnImageFiles}
            COMMENT "AI_CMAKE_STATUS [COMPILE_RESOURCES] platform='MAC' addon='${addOnName}' language='${addOnLanguage}'"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${ResourceObjectsDir}"
            COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/CompileResources.py" "${addOnLanguage}" "${devKitDir}" "${AddOnSourcesFolderAbsolute}" "${AddOnResourcesFolderAbsolute}" "${ResourceObjectsDir}" "${CMAKE_BINARY_DIR}/$<CONFIG>/${addOnName}.bundle/Contents/Resources"
            COMMAND ${CMAKE_COMMAND} -E copy "${devKitDir}/Inc/PkgInfo" "${CMAKE_BINARY_DIR}/$<CONFIG>/${addOnName}.bundle/Contents/PkgInfo"
            COMMAND ${CMAKE_COMMAND} -E touch ${ResourceStampFile}
        )
    endif ()

    file (GLOB_RECURSE AddOnHeaderFiles CONFIGURE_DEPENDS
        ${addOnSourcesFolder}/*.h
        ${addOnSourcesFolder}/*.hpp
    )
    # Только *.cpp: под api_headers/ лежат все восемь версий APICommon22.c…APICommon29.c
    # с одними и теми же символами (WriteReport, ErrID_To_Name, ...), и глоб по *.c
    # дал бы LNK2005/LNK2019 на восьмикратном дублировании. Версию выбирает
    # -DAC_${acVersion} через api_headers/APIEnvir.h, в target им попадать не надо.
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

    # VERSIONINFO-ресурс .apx (свойства файла) и STRS 18000 (код языка ресурсов).
    # На Windows только — на macOS эти же данные уходят в Info.plist ниже.
    # ADDON_VERSION не переопределяем: semver здесь с тремя компонентами
    # ("1.78.0" из 1;78;0), а в UI по решению #214 показывается версия из
    # config.json как есть ("1.78"). Три компонента нужны только FILEVERSION.
    generate_add_on_version_info (${addOnName} ${acVersion} unusedSemver)

    # Хэш коммита в подверсию: по строке версии в grc/plist видно, из какой
    # ревизии собран аддон. Хэш читается на этапе configure, поэтому без
    # повторного configure он не обновляется после новых коммитов.
    find_package (Git QUIET)
    if (GIT_FOUND)
        execute_process (
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE gitCommitHash
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE gitResult
        )
        if (NOT gitResult EQUAL 0)
            set (gitCommitHash "unknown")
        endif ()
    else ()
        set (gitCommitHash "unknown")
    endif ()
    message (STATUS "Building from commit: ${gitCommitHash}")

    string(TIMESTAMP addonsubversion "%Y-%m-%d ")
    set(ADDON_SUBVERSION "${addonsubversion} #${gitCommitHash}")
    
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
        # AddOn.plist.in подставляет это значение в LSMinimumSystemVersion:
        # раньше там был хардкод 10.15, из-за чего значение вычислялось вхолостую
        # и расходилось с требованиями DevKit (для AC28/29 нужно 11.0).
        set(MINIMUM_SYSTEM_VERSION "${lsMinimumSystemVersion}")
        # Идентификатор bundle обязан совпадать с CFBundleIdentifier в AddOn.plist.in.
        # Считаем здесь из addOnName, а в plist подставляем @bundleIdentifier@, иначе
        # значения в двух местах разъезжаются (было: com.kuvbur. + неопределённый
        # addOnNameIdentifier, то есть обрезанный префикс). Идентичность аддона
        # не меняется — префикс com.graphisoft.addon. сохранён.
        set(bundleIdentifier "com.graphisoft.addon.${addOnName}")

        configure_file(
                "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/AddOn.plist.in"
                "${CMAKE_CURRENT_LIST_DIR}/${addOnResourcesFolder}/RFIX.mac/Info.plist"
                @ONLY
            )

        set_target_properties(${addOnName} PROPERTIES
            BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/${addOnResourcesFolder}/RFIX.mac/Info.plist"

            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${bundleIdentifier}
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
    )
    target_include_directories (${addOnName} SYSTEM PUBLIC ${devKitDir}/Inc)

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
