DOCUMENTATION ON HOW TO UPLOAD THE CODE VIA OTA AFTER UPLOADING IT USING USB (ENTERPRISE NETWORKS):

YOU CAN DIRECTLY USE UPLOAD BUTTON IF YOU ARE USING LOCAL NETWORK

1) After you made your changes, go to Sketch > Export Compiled Binary on Arduino IDE or 
build the project in PlatformIO to get a file like {file_name}.ino.bin

2) In case of Arduino IDE you have to go inside build folder to find the required file. In PlatformIO find the bin file inside pio/build

3) Locate espota.py using find ~ -name espota.py

4) Run python3 {path_to_espota.py} -i {your_wifi_IP} -p 3232 -auth={your_OTA_Password} --file {path_to_the_bin_file}

YOU CAN ALSO FIND THE VIDEO IN media