import os

Import("env")

custom_src = env.GetProjectOption("custom_src_dir")
full_path = os.path.join("$PROJECT_DIR", custom_src)

print("\nPROJECT SOURCE DIR: " + full_path)

env['PROJECT_SRC_DIR'] = full_path

# print(env.Dump())
