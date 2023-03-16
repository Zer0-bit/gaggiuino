# 09:32 15/03/2023 - change triggering comment
import subprocess

Import("env")

def get_firmware_version_build_flag():
  ret = subprocess.run(["git", "describe", "--always"], stdout=subprocess.PIPE, text=True)

  if ret.returncode == 0:
    build_version = ret.stdout.strip()
  else:
    build_version = ""
    try:
      with open('VERSION') as f:
        build_version = f.readline().strip()
    except:
        pass

    if build_version == "":
      build_version = "nogit"

  build_flag = "-D AUTO_VERSION=\\\"" + build_version + "\\\""
  print ("\nFIRMWARE VERSION: " + build_version)
  return (build_flag)

env.Append(
  BUILD_FLAGS=[get_firmware_version_build_flag()]
)
