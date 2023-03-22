import sys, os

# Usage: composite_video.py [video1] [video2] [video3] [video4] [output_video]
assert(len(sys.argv) == 6)
inputs = ''
for i in range(1, 5):
    inputs += '-i ' + sys.argv[i] + ' '
output = sys.argv[5]

# Call ffmpeg to composite videos in a 2x2 grid
args = ' -filter_complex "[0:v][1:v]hstack=inputs=2:shortest=1[top]; [2:v][3:v]hstack=inputs=2:shortest=1[bottom]; [top][bottom]vstack=inputs=2:shortest=1[v]" -map "[v]" '
os.system('ffmpeg ' + inputs + args + output)