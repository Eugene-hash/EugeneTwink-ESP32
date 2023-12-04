#ifndef CONST_H
#define CONST_H

#define MAIN_MODE           0b00000001
#define MAIN_BRIGHT         0b00000010
#define MAIN_SWITCH         0b00000100
#define MAIN_SETTING        0b00000000

#define SWITCH_NONE         0b00000000
#define SWITCH_POWER        0b00000001
#define SWITCH_RANDOM       0b00000010
#define SWITCH_AUTOPLAY     0b00000100

#define CAMERA_OPEN         0b00010000
#define CAMERA_CLOSE        0b00100000
#define CAMERA_SCAN_NEXT    0b00000000
#define CAMERA_SCAN_STOP    0b00000001
#define CAMERA_SCAN_START   0b01000000

#endif