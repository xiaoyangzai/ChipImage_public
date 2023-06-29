import sys
import cv2
from img_proc import *


def main():
    img = cv2.imread(sys.argv[1])
    print(stat_brightness_mean(img))
    print(stat_brightness_RMS(img))
    print(stat_brightness_formula(img))
    print(stat_brightness_RMS_formula(img))


if __name__ == '__main__':
    main()
