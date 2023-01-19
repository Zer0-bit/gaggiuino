from sys import platform
import os
Import("env")

if platform == "linux" or platform == "linux2" or platform == "darwin":
    print("Replace MKSPIFFSTOOL with mklittlefs")
    env['MKSPIFFSTOOL'] = "mklittlefs"
else:
    print("Replace MKSPIFFSTOOL with mklittlefs.exe")
    env['MKSPIFFSTOOL'] = "mklittlefs.exe"
