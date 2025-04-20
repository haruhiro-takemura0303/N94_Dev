# Copyright 2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause 

import cv2
import math
import numpy as np

desired_width   = 96
desired_height  = 39
threshold = 200

image_root = input("Image root: ")

img = cv2.imread(image_root)

grayimg = cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)

img_shape = img.shape

width   = img_shape[1]
height  = img_shape[0]

width_scale_factor = width/desired_width
height_scale_factor = height/desired_height

new_width = 0
new_height = 0

print("-------------------------------------------")
print("Original width: ", width)
print("Original Height: ", height)
print("-------------------------------------------")
print("Desired width: ", desired_width)
print("Desired Height: ", desired_height)
print("-------------------------------------------")
print("Width scale factor", width_scale_factor)
print("Height scale factor", height_scale_factor)
print("-------------------------------------------")

if(height_scale_factor > width_scale_factor):
    print("Height scale factor selected")
    new_width = width / height_scale_factor
    new_height = height / height_scale_factor
else:
    print("Width scale factor selected")
    new_width = width / width_scale_factor
    new_height = height / width_scale_factor

print("New width: ", math.ceil(new_width))
print("New height: ", math.ceil(new_height))
print("-------------------------------------------")

resized_img = cv2.resize(grayimg, (math.ceil(new_width), math.ceil(new_height)))

img_complete = np.zeros((desired_height, desired_width))

error_width = desired_width - new_width
error_height = desired_height - new_height

offset_width = math.floor(error_width/2)
offset_heigth = math.floor(error_height/2)

for row in range(0, math.ceil(new_height)):
    for column in range(0, math.ceil(new_width)):
        img_complete[row+offset_heigth][column+offset_width] = resized_img[row][column]

#cv2.imshow('image', resized_img)
boolean_in8bit = list()

count = 0
val = 0

page_l = list()
img_8bits = list()

for page in range(0,math.ceil(desired_height/8)):
    for column in range(0, math.ceil(desired_width)):
        count = 0
        val = 0
        for row in range(page*8, (page+1)*8):
            if(row > (desired_height-1)):
                row = desired_height-1
            if(img_complete[row][column] > threshold):
                val = val | (1<<count)
            count = count + 1
        page_l.append(val)
    img_8bits.append(page_l[math.ceil(desired_width)*page:math.ceil(desired_width)*(page+1)])            


# Write file -------------------------------------------------------------------------------------
img_bin = open("img_bin.c", "w")

img_bin.write("const uint8_t g_image["+str(count)+"]["+str(desired_width)+"] = \n{\n")

for rows in img_8bits:
    img_bin.write("\t{")
    count = 0
    for bytes in rows:
        img_bin.write(str(bytes) + ", ")
        count = count + 1
        if(count == 20):
            count = 0
            img_bin.write("\n\t")
    img_bin.write("},\n\n")
img_bin.write("};")
img_bin.close()
# -------------------------------------------------------------------------------------------------
cv2.waitKey(0)      