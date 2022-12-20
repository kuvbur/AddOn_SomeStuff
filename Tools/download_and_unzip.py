import os
import sys
import subprocess
import platform
import zipfile
import requests
API_ENDPOINT = 'https://cloud-api.yandex.net/v1/disk/public/resources/download?public_key={}'

url = sys.argv[1]
dest = sys.argv[2]

if not os.path.exists (dest):
    os.makedirs (dest)

def _get_real_direct_link(sharing_link):
    pk_request = requests.get(API_ENDPOINT.format(sharing_link))
    return pk_request.json().get('href')

def _extract_filename(direct_link):
    for chunk in direct_link.strip().split('&'):
        if chunk.startswith('filename='):
            return chunk.split('=')[1]
    return None

def download_yadisk_link(sharing_link):
    direct_link = _get_real_direct_link(sharing_link)
    if direct_link:
        filename = _extract_filename(direct_link)
        filePath = os.path.join(dest, filename)
        download = requests.get(direct_link)
        with open(filePath, 'wb') as out_file:
            out_file.write(download.content)
        print('Downloaded "{}" to "{}"'.format(sharing_link, filePath))
        return filePath
    else:
        print('Failed to download "{}"'.format(sharing_link))
        return None

filePath = download_yadisk_link(url)

if filePath is not None:
    if platform.system () == 'Windows':
        with zipfile.ZipFile (filePath, 'r') as zip:
            zip.extractall (dest)
    elif platform.system () == 'Darwin':
        subprocess.call ([
            'unzip', '-qq', filePath,
            '-d', dest
        ])

sys.exit (0)
