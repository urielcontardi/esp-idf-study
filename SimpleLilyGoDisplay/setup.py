# Fixed to ESP32 TDisplay
import os
import shutil

current_directory = os.getcwd()
sdkconfig_path = os.path.join(current_directory, "sdkconfig")
output_path = os.path.join(current_directory, "build")
idf_component_path = os.path.join(current_directory, "main")
idf_component_path = os.path.join(idf_component_path, "idf_component.yml")

os.system("git submodule init")
os.system("git submodule update")

if os.path.isfile(idf_component_path):
    os.remove(idf_component_path)
if os.path.isfile(sdkconfig_path):
    print("Delete temporary files")
    os.remove(sdkconfig_path)
if os.path.exists(output_path):
    print("Delete temporary files")
    shutil.rmtree(output_path)

r = os.system("idf.py > " + os.devnull)
if r != 0:
    print(
        "Unable to execute idf.py, please see here to learn how to use and install https://github.com/espressif/esp-idf"
    )
    exit()


def copy_file(source_path, destination_path):
    try:
        shutil.copy(source_path, destination_path)
        print(f"Set default compilation configuration")
    except FileNotFoundError:
        print("File not found, please check the file path.")
    except PermissionError:
        print("No permission to access file, please check file permissions.")
    except Exception as e:
        print(f"An error occurred: {e}")

def perform_action():
    os.system('idf.py add-dependency "lvgl/lvgl^8.3.11"')

    os.system("idf.py reconfigure")

    os.system("idf.py build")

if __name__ == "__main__":
    print("Fixed to ESP32 TDisplay")
    perform_action()
