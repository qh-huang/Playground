#!/usr/bin/env python

# make it compatible to both Python 2 and 3
from __future__ import division, print_function
from builtins import bytes, chr, int
from future.utils import raise_with_traceback, with_metaclass

import os
import requests
from clint.textui import progress
from subprocess import check_call

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
DATASETS_ROOT = SCRIPT_DIR
ADVIO_DIR = DATASETS_ROOT + "/advio"
ADVIO_URL_PREFIX = "https://zenodo.org/record/1476931/files/advio-"


def download_file(url, dst):
    local_filename = url.split('/')[-1]
    with requests.get(url, stream=True) as r:
        r.raise_for_status()
        file_path = dst + "/" + local_filename
        with open(file_path, 'wb') as f:
            print("downloading file: " + file_path)
            total_length = int(r.headers.get('content-length'))
            for chunk in progress.bar(r.iter_content(chunk_size=1024), expected_size=(total_length/1024) + 1):
                if chunk:
                    f.write(chunk)
                    f.flush()
    return local_filename


def main(argv):
    if not os.path.exists(ADVIO_DIR):
        os.mkdir(ADVIO_DIR)
    for i in range(1, 24):
        file_url = ADVIO_URL_PREFIX + "{:02d}".format(i) + ".zip"
        downloaded_file = download_file(file_url, ADVIO_DIR)

    os.sys.exit(0)


if __name__ == '__main__':
    main(os.sys.argv[1:])
