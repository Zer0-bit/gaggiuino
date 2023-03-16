# 09:32 15/03/2023 - change triggering comment
from sys import platform
import os
Import("env")

custom_src = env.GetProjectOption("custom_src_dir")

project_dir = "$PROJECT_DIR"

if platform == "linux" or platform == "linux2" or platform == "darwin":
  project_dir = os.path.join("//", project_dir)

full_path = os.path.join(project_dir, custom_src)
full_data_path = os.path.join(project_dir, full_path, "data")

print("\nPROJECT SOURCE DIR: " + full_path)
print("\nPROJECT DATA DIR: " + full_data_path)

env['PROJECT_DATA_DIR'] = full_data_path
env['PROJECT_SRC_DIR'] = full_path

# print(env.Dump())
