import cv2
import numpy as np


def stat_brightness_mean(img):
    if img.shape[-1] == 3:
        img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    img = img.astype(float)
    return img.mean()


def stat_brightness_RMS(img):
    if img.shape[-1] == 3:
        img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    img = img.astype(float)
    brightness = np.sqrt((img*img).mean())
    return brightness


def stat_brightness_formula(img):
    if img.shape[-1] == 1:
        img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
    rgb = img.astype(float).mean(0).mean(0)[::-1]
    brightness = np.sqrt((0.241 * rgb[0]**2 + 0.691 * rgb[1]**2 + 0.068 * rgb[2]**2))
    return brightness


def stat_brightness_RMS_formula(img):
    if img.shape[-1] == 1:
        img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
    img = img.astype(float)
    rms = (img*img).mean(0).mean(0)[::-1]
    brightness = np.sqrt((0.241 * rms[0] + 0.691 * rms[1] + 0.068 * rms[2]))
    return brightness
