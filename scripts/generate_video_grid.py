import cv2 
import numpy as np
import glob, os, sys

# Usage: generate_video.py [video_dir1] [video_dir2] [video_dir3] [video_dir4] [output]
folders = sys.argv[1:5]
output = sys.argv[5]

# Get all frame names (assuming they're common across all folders)
frames = []
for filename in sorted(glob.glob(os.path.join(folders[0],"*.png"))):
    frame = os.path.basename(filename)
    frames.append(frame)

# Load one image to get the frame dimensions
example_image = cv2.imread(os.path.join(folders[0], frames[0]))
height, width, channels = example_image.shape

# Specify the text to add to the videos
text_coords = (50, height - 50)
video_text = ["top-left", "top-right", "bottom-left", "bottom-right"]

# Write out a grid of frames
fps = 20
video_size = (2 * width, 2 * height)
video_writer = cv2.VideoWriter(output, cv2.VideoWriter_fourcc(*'DIVX'), fps, video_size)
for frame in frames:
    # Read in the appropriate frame from each folder
    image_grid = []
    for i in range(4):
        image_path = os.path.join(folders[i], frame)
        image = cv2.imread(image_path)
        image = cv2.putText(image, video_text[i], text_coords, cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2, cv2.LINE_AA)
        image_grid.append(image)
    
    # Put the images together in a grid
    top = np.concatenate((image_grid[0], image_grid[1]), axis=1)
    bot = np.concatenate((image_grid[2], image_grid[3]), axis=1)
    final = np.concatenate((top, bot), axis=0)
    video_writer.write(final)
    print("Wrote frame", frame)

video_writer.release()