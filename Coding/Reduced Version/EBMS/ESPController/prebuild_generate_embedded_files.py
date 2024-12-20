""" prebuild_generate_embedded_files for EBMS """
import datetime
import subprocess
import os
from os import path
import struct
import glob
import hashlib

Import("env")

def prepare_embedded_files(data_dir, include_dir, filenamePrefix):
    #This routine takes every file in the web_temp folder and converts
    #to a byte array suitable for embedding into flash to avoid using SPIFF or LITTLEFS
    #it also generates checksums and file length variables to improve caching and speed

    print('prebuild_generate_embedded_files.py')

    if (os.path.exists(data_dir)==False or os.path.exists(include_dir)==False):
        raise Exception("Missing project folder - data or include folder")
 
    all_files = glob.glob(os.path.join(data_dir, '*.*'))

    sha1sum = hashlib.sha1()

    with open(os.path.join(include_dir,filenamePrefix+'_Blobs.h'), 'w') as blobs,  open(os.path.join(include_dir,filenamePrefix+'.h'), 'w') as f:
        blobs.write("// This is an automatically generated file, any changes will be overwritten on compiliation!\n")

        f.write("// This is an automatically generated file, any changes will be overwritten on compiliation!\n")
        f.write("\n\n#ifndef "+filenamePrefix+"_H\n#define "+filenamePrefix+"_H\n\n")
        f.write("\n#include \""+filenamePrefix+"_Blobs.h\"\n");

        for file in all_files:
            print("Embedding {}".format(file))

            blobs.write("//{}\n".format(file))
            name="file_"+os.path.basename(file).replace(".", "_")

            # Generate SHA1 hash of the file for the ETAG HTTP header            
            with open(file, 'rb') as source:
                block = source.read(2**16)
                while len(block) != 0:
                    sha1sum.update(block)
                    block = source.read(2**16)
                etag=sha1sum.hexdigest()

            f.write("const char* const etag_{} = \"\\\"{}\\\"\";\n".format(name,etag[32:]))

            # Generate constant variable to hold size of this file/byte array
            filelength=os.stat(file).st_size
            f.write("const size_t size_{} = {};\n\n".format(name, filelength))

            rawfile = open(file, "rb")

            blobs.write("const uint8_t {}[] PROGMEM = {{\n".format(name))

            while True:
                block = rawfile.read(16)
                if len(block) < 16:
                    if len(block):
                        blobs.write("\t")
                        for b in block:
                            # Python 2/3 compat
                            if type(b) is str:
                                b = ord(b)
                            blobs.write("0x{:02X}, ".format(b))
                        blobs.write("\n")
                    break
                blobs.write("\t0x{:02X}, 0x{:02X}, 0x{:02X}, 0x{:02X}, "
                        "0x{:02X}, 0x{:02X}, 0x{:02X}, 0x{:02X}, "
                        "0x{:02X}, 0x{:02X}, 0x{:02X}, 0x{:02X}, "
                        "0x{:02X}, 0x{:02X}, 0x{:02X}, 0x{:02X},\n"
                        .format(*struct.unpack("BBBBBBBBBBBBBBBB", block)))

            # Add trailing ZERO to text based files
            if file.endswith(".htm"):
                blobs.write("0x00\n")

            blobs.write("};\n")

            rawfile.close

        f.write("#endif")


prepare_embedded_files(os.path.join(env.get('PROJECT_DIR'), 'web_temp'), os.path.join(env.get('PROJECT_DIR'), 'include'), "EmbeddedFiles_AutoGenerated")


#prepare_embedded_files(os.path.join(env.get('PROJECT_DIR'), 'avr_firmware'), os.path.join(env.get('PROJECT_DIR'), 'include'), "AVRFirmware_AutoGenerated")
