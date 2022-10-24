import os

Import("env")

custom_src = env.GetProjectOption("custom_src_dir")
full_path = os.path.join("$PROJECT_DIR", custom_src)
full_data_path = os.path.join("$PROJECT_DIR", full_path + "//data")

print("\nPROJECT SOURCE DIR: " + full_path)
print("\nPROJECT DATA DIR: " + full_data_path)

env['PROJECT_DATA_DIR'] = full_data_path
env['PROJECT_SRC_DIR'] = full_path

# print(env.Dump())
