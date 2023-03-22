import cv2 
import glob, os, sys

# Usage: generate_video.py [image_dir] [output_video]
folder = sys.argv[1]
output = sys.argv[2]

# Add all frames to an array (assuming frames are named in alphabetical order)
img_array = []
for filename in sorted(glob.glob(os.path.join(folder,"*.png"))):
    print(filename)
    img = cv2.imread(filename)
    height,width,layers = img.shape
    size = (width,height)
    img_array.append(img)

# Write out video after all frames have been read
fps = 20
out = cv2.VideoWriter(output, cv2.VideoWriter_fourcc(*'DIVX'), fps, size)
for i in range(len(img_array)):
    out.write(img_array[i])
out.release()