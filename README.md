# BPI-A83T-Android6
BPI-M3 Android6.0
----------
1 Build Android BSP

 $ cd lichee
 
 $ ./build.sh config       

Welcome to mkscript setup progress
All available chips:
   1. sun8iw6p1

Choice: 1


All available platforms:
   1. android
   2. dragonboard
   3. linux
   4. camdroid
   5. secureandroid
 
Choice: 1


All available kernel:
   1. linux-3.4
 
Choice: 1


All available boards:
   1. bpi-m3-hdmi
   2. bpi-m3-lcd5
   3. bpi-m3-lcd7

Choice: 1

   $ ./build.sh 

***********

2 Build Android 

   $cd ../android

   $source build/envsetup.sh
   
   $lunch    //(bpi_m3_hdmi-eng)
   
   $extract-bsp
   
   $make -j8
   
   $pack
