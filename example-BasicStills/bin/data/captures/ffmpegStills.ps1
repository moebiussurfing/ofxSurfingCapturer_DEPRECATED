# covert tif stills to mp4 video

# https://hamelot.io/visualization/using-ffmpeg-to-convert-a-set-of-images-into-a-video/

# ffmpeg -r 60 -f image2 -s 1920x1080 -i pic%04d.png -vcodec libx264 -crf 25  -pix_fmt yuv420p test.mp4
# 
# where the %04d means that zeros will be padded until the length of the string is 4 i.e 0001…0020…0030…2000 and so on. If no padding is needed use something similar to pic%d.png or %d.png.
# 
#     -r is the framerate (fps)
#     -crf is the quality, lower means better quality, 15-25 is usually good
#     -s is the resolution
#     -pix_fmt yuv420p specifies the pixel format, change this as needed
# 
# the file will be output (in this case) to: test.mp4

# https://ponchomx.wordpress.com/2011/04/26/aprendiendo-a-usar-ffmpeg/
# https://trac.ffmpeg.org/wiki
# https://trac.ffmpeg.org/wiki/Encode/H.265

# ! https://trac.ffmpeg.org/wiki/Encode/H.265
# ! https://www.ffmpeg.org/ffmpeg.html#toc-Description
# https://superuser.com/questions/1296374/best-settings-for-ffmpeg-with-nvenc

# ffmpeg -r 60 -f image2 -s 1920x1080 -i %05d.tif -vcodec libx264 -crf 25  -pix_fmt yuv420p test.mp4
# ffmpeg -r 60 -f image2 -s 1920x1080 -i %05d.tif -vcodec libx264 -crf 25 -b:v 6000k -pix_fmt yuv420p test.mp4
# ffmpeg -r 60 -f image2 -s 1920x1080 -i %05d.tif -vcodec libx264 -crf 28 -b:v 6000k -pix_fmt yuv420p test.mp4

# loss-less
# https://stackoverflow.com/questions/11713882/lossless-ffmpeg-conversion-encoding
# https://forum.videohelp.com/threads/352346-ffmpeg-how-to-lossless-convert-video-codec-to-libx264-CRF-qp-value

ffmpeg -r 60 -f image2 -s 1920x1080 -i %05d.tif -c:v libx264 -preset veryslow -qp 0 output.mp4