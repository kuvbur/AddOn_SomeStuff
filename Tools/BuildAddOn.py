import argparse
import json
import os
import pathlib
import platform
import shutil
import subprocess
import sys
import urllib.parse
import urllib.request
import zipfile
import requests
API_ENDPOINT = 'https://cloud-api.yandex.net/v1/disk/public/resources/download?public_key={}'


def _extract_filename_yadisk_link(direct_link):
    for chunk in direct_link.strip().split('&'):
        if chunk.startswith('filename='):
            return chunk.split('=')[1]
    return None


def DownloadFromYadisk(url, dest):
    pk_request = requests.get(API_ENDPOINT.format(url))
    direct_link = pk_request.json().get('href')
    if direct_link:
        filename = _extract_filename_yadisk_link(direct_link)
        filePath = os.path.join(dest, filename)
        download = requests.get(direct_link)
        with open(filePath, 'wb') as out_file:
            out_file.write(download.content)
        print('Downloaded "{}" to "{}"'.format(url, filePath))
        return filePath
    else:
        print('Failed to download "{}"'.format(url))
        return None


def ParseArguments():
    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--configFile', dest='configFile',
                        required=True, help='JSON Configuration file')
    parser.add_argument('-v', '--acVersion', dest='acVersion', nargs='+',
                        type=str, required=False, help='Archicad version number list. Ex: 26 27')
    parser.add_argument('-l', '--language', dest='language', nargs='+', type=str, required=False,
                        help='Add-On language code list. Ex: INT GER. Specify ALL for all languages in the configfile.')
    parser.add_argument('-d', '--devKitPath', dest='devKitPath',
                        type=str, required=False, help='Path to local APIDevKit')
    parser.add_argument('-r', '--release', dest='release', required=False,
                        action='store_true', help='Build in localized Release mode.')
    parser.add_argument('-p', '--package', dest='package', required=False,
                        action='store_true', help='Create zip archive.')
    args = parser.parse_args()

    if args.devKitPath is not None:
        if args.acVersion is None:
            raise Exception(
                'Must provide one Archicad version with local APIDevKit option!')
        if len(args.acVersion) != 1:
            raise Exception(
                'Only one Archicad version supported with local APIDevKit option!')

    return args


def PrepareParameters(args):
    # Check platform operating system
    platformName = None
    if platform.system() == 'Windows':
        platformName = 'WIN'
    elif platform.system() == 'Darwin':
        platformName = 'MAC'
    # Load config data
    configPath = pathlib.Path(args.configFile)
    if configPath.is_dir():
        raise Exception(f'{configPath} is a directory!')
    configFile = open(configPath)
    configData = json.load(configFile)
    addOnName = configData['addOnName']
    acVersionList = None
    languageList = None

    if args.acVersion:
        acVersionList = args.acVersion
    else:
        acVersionList = []
        for version in configData['devKitLinks'][platformName]:
            acVersionList.append(version)

    # Get needed language codes
    if args.release:
        configLangUpper = [lang.upper() for lang in configData['languages']]
        languageList = ['ALL']
        if args.language is not None:
            languageList = [lang.upper() for lang in args.language]

        if 'ALL' in languageList:
            languageList = configLangUpper
        else:
            for lang in languageList:
                if lang not in configLangUpper:
                    raise Exception('Language not supported!')

    return [configData, platformName, addOnName, acVersionList, languageList]


def PrepareDirectories(args, configData, platformName, addOnName, acVersionList):
    # Create directory for Build and Package
    workspaceRootFolder = pathlib.Path(
        __file__).parent.absolute().parent.absolute()
    buildFolder = workspaceRootFolder / 'Build'
    packageRootFolder = buildFolder / 'Package' / addOnName
    devKitFolderList = {}

    if not buildFolder.exists():
        buildFolder.mkdir(parents=True)

    if args.package:
        if (packageRootFolder).exists():
            shutil.rmtree(packageRootFolder)

    # Set APIDevKit directory if local is used, else create new directories
    if args.devKitPath is not None:
        devKitPath = pathlib.Path(args.devKitPath)
        if not devKitPath.is_dir():
            raise Exception(f'{devKitPath} is not a directory!')
        devKitFolderList[acVersionList[0]] = devKitPath
    else:
        # For every ACVersion
        # Check if APIDevKitLink is provided
        # Create directory for APIDevKit
        # Download APIDevKit
        for version in acVersionList:
            if version in configData['devKitLinks'][platformName]:

                devKitFolder = buildFolder / 'DevKit' / f'APIDevKit-{version}'
                if not devKitFolder.exists():
                    devKitFolder.mkdir(parents=True)

                devKitFolderList[version] = devKitFolder
                DownloadAndUnzip(
                    configData['devKitLinks'][platformName][version], devKitFolder)

            else:
                raise Exception('APIDevKit download link not provided!')

    return [workspaceRootFolder, buildFolder, packageRootFolder, devKitFolderList]


def DownloadAndUnzip(url, dest):
    if 'disk.yandex' in url:
        filePath = DownloadFromYadisk(url, dest)
        fileName = filePath.split('/')[-1]
    else:
        fileName = url.split('/')[-1]
        filePath = pathlib.Path(dest, fileName)
        if filePath.exists():
            return
        print(f'Downloading {fileName}')
        urllib.request.urlretrieve(url, filePath)

    print(f'Unzipping {fileName}')
    if platform.system() == 'Windows':
        with zipfile.ZipFile(filePath, 'r') as zip:
            zip.extractall(dest)
    elif platform.system() == 'Darwin':
        subprocess.call([
            'unzip', '-qq', filePath,
            '-d', dest
        ])


def GetInstalledVisualStudioGenerator():
    vsWherePath = pathlib.Path(
        os.environ["ProgramFiles(x86)"]) / 'Microsoft Visual Studio' / 'Installer' / 'vswhere.exe'
    if not vsWherePath.exists():
        raise Exception('Microsoft Visual Studio Installer not found!')
    vsWhereOutputStr = subprocess.check_output(
        [vsWherePath, '-sort', '-format', 'json']).decode('utf-8', 'ignore')
    vsWhereOutput = json.loads(vsWhereOutputStr)
    if len(vsWhereOutput) == 0:
        raise Exception('No installed Visual Studio detected!')
    vsVersion = vsWhereOutput[0]['installationVersion'].split('.')[0]
    if vsVersion == '17':
        return 'Visual Studio 17 2022'
    elif vsVersion == '16':
        return 'Visual Studio 16 2019'
    else:
        raise Exception('Installed Visual Studio version not supported!')


def GetProjectGenerationParams(workspaceRootFolder, buildPath, platformName, devKitFolder, version, languageCode, optionalParams):
    # Add params to configure cmake
    projGenParams = [
        'cmake',
        '-B', str(buildPath)
    ]

    if platformName == 'WIN':
        vsGenerator = GetInstalledVisualStudioGenerator()
        projGenParams.append(f'-G {vsGenerator}')
        toolset = 'v142'
        if int(version) < 25:
            toolset = 'v141'
        if int(version) < 23:
            toolset = 'v140'
        projGenParams.append(f'-T {toolset}')
    elif platformName == 'MAC':
        projGenParams.extend(['-G', 'Xcode'])

    projGenParams.append(
        f'-DAC_API_DEVKIT_DIR={str (devKitFolder / "Support")}')

    if languageCode is not None:
        projGenParams.append(f'-DAC_ADDON_LANGUAGE={languageCode}')

    if optionalParams is not None:
        for key in optionalParams:
            projGenParams.append(f'-D{key}={optionalParams[key]}')

    projGenParams.append(str(workspaceRootFolder))

    return projGenParams


def BuildAddOn(configData, platformName, workspaceRootFolder, buildFolder, devKitFolder, version, configuration, languageCode=None):
    addOnName = configData['addOnName']
    optionalParams = None
    if 'addOnSpecificCMakeParameterEnvVars' in configData:
        optionalParams = {}
        for key in configData['addOnSpecificCMakeParameterEnvVars']:
            value = os.environ.get(key)
            if value is None:
                raise Exception(f'{key} - Environment variable not found!')
            optionalParams[key] = value

    buildPath = buildFolder / addOnName / version
    if languageCode is not None:
        buildPath = buildPath / languageCode

    # Add params to configure cmake
    projGenParams = GetProjectGenerationParams(
        workspaceRootFolder, buildPath, platformName, devKitFolder, version, languageCode, optionalParams)
    projGenResult = subprocess.call(projGenParams)
    if projGenResult != 0:
        raise Exception('Failed to generate project!')

    # Add params to build AddOn
    buildParams = [
        'cmake',
        '--build', str(buildPath),
        '--config', configuration
    ]
    buildResult = subprocess.call(buildParams)
    if configuration == 'Debug':
        shutil.copy(
            workspaceRootFolder / f'Test_file/test_{version}.pln',
            buildPath / f'test_{version}.pln',
        )
    if buildResult != 0:
        raise Exception('Failed to build project!')


def BuildAddOns(args, configData, platformName, languageList, workspaceRootFolder, buildFolder, devKitFolderList):
    # At this point, devKitFolderList dictionary has all provided ACVersions as keys
    # For every ACVersion
    # If release, build Add-On for all languages with RelWithDebInfo configuration
    # Else build Add-On with Debug and RelWithDebInfo configurations, without language specified
    # In each case, if package creation is enabled, copy the .apx/.bundle files to the Package directory
    try:
        for version in devKitFolderList:
            devKitFolder = devKitFolderList[version]
            if args.release is True:
                for languageCode in languageList:
                    BuildAddOn(configData, platformName, workspaceRootFolder, buildFolder,
                               devKitFolder, version, 'RelWithDebInfo', languageCode)

            else:
                BuildAddOn(configData, platformName, workspaceRootFolder,
                           buildFolder, devKitFolder, version, 'Debug')

    except Exception as e:
        raise e


def Check7ZInstallation():
    try:
        subprocess.call('7z', stdout=subprocess.DEVNULL)
    except:
        raise Exception('7Zip not installed!')


def CopyResultToPackage(packageRootFolder, buildFolder, version, addOnName, platformName, configuration, languageCode=None, isRelease=False):
    packageFolder = packageRootFolder / version
    sourceFolder = buildFolder / addOnName / version

    if languageCode is not None:
        # packageFolder = packageFolder / languageCode
        sourceFolder = sourceFolder / languageCode
    sourceFolder = sourceFolder / configuration

    if not packageFolder.exists():
        packageFolder.mkdir(parents=True)

    if platformName == 'WIN':
        shutil.copy(
            sourceFolder / f'{addOnName}.apx',
            packageFolder / f'{addOnName}_{version}.apx',
        )
        if configuration == 'Debug':
            shutil.copy(
                sourceFolder / f'{addOnName}.pdb',
                packageFolder / f'{addOnName}_{version}.pdb',
            )

    elif platformName == 'MAC':
        subprocess.call([
            'cp', '-r',
            sourceFolder / f'{addOnName}.bundle',
            packageFolder / f'{addOnName}.bundle'
        ])


# Zip packages
def PackageAddOns(args, addOnName, platformName, acVersionList, languageList, buildFolder, packageRootFolder):
    # Check7ZInstallation()
    for version in acVersionList:
        if args.release:
            for languageCode in languageList:
                CopyResultToPackage(packageRootFolder, buildFolder, version,
                                    addOnName, platformName, 'RelWithDebInfo', languageCode, True)
        else:
            CopyResultToPackage(packageRootFolder, buildFolder,
                                version, addOnName, platformName, 'Debug')
            CopyResultToPackage(packageRootFolder, buildFolder,
                                version, addOnName, platformName, 'RelWithDebInfo')

        # buildType = 'Release' if args.release else 'Daily'
        # subprocess.call([
        #     '7z', 'a',
        #     str(packageRootFolder.parent /
        #         f'{addOnName}_AC{version}_{platformName}.zip'),
        #     str(packageRootFolder / version / '*')
        # ])


def Main():
    try:
        args = ParseArguments()

        [configData, platformName, addOnName, acVersionList,
            languageList] = PrepareParameters(args)

        [workspaceRootFolder, buildFolder, packageRootFolder, devKitFolderList] = PrepareDirectories(
            args, configData, platformName, addOnName, acVersionList)
        os.chdir(workspaceRootFolder)

        BuildAddOns(args, configData, platformName, languageList,
                    workspaceRootFolder, buildFolder, devKitFolderList)

        if args.package:
            PackageAddOns(args, addOnName, platformName, acVersionList,
                          languageList, buildFolder, packageRootFolder)
        print('Build succeeded!')
        sys.exit(0)

    except Exception as e:
        print(e)
        sys.exit(1)


if __name__ == "__main__":
    Main()
