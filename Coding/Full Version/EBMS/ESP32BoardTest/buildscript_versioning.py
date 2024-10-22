""" Script for EBMS """
import datetime
import subprocess
import os
from os import path

Import("env")

env.Replace(PROGNAME="Ebms_boardtest_%s_%s" %
            (env["PIOPLATFORM"], env["PIOENV"]))

env.Replace(ESP32_SPIFFS_IMAGE_NAME="Ebms_boardtest_fsi_%s_%s" %
            (env["PIOPLATFORM"], env["PIOENV"]))

