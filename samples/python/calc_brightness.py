import sys
import cv2
from img_proc import *


def main():
    img = cv2.imread(sys.argv[1])
    print('calculating brightness:')
    print(stat_brightness_mean(img))
    print(stat_brightness_RMS(img))
    print(stat_brightness_formula(img))
    print(stat_brightness_RMS_formula(img))

    print('calculating sharpness:')
    print(stat_sharpness_Tenengrad(img, threshold=100))
    print(stat_sharpness_Laplacian(img))
    print(stat_sharpness_Variance(img))


if __name__ == '__main__':
    main()
