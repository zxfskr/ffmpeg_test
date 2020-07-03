# 视频加密


```
rm -rf *.ts *.m3u8


ffmpeg -y -i pikachu.mp4 -c:v libx264 -c:a libfdk_aac -bsf:v h264_mp4toannexb -map 0 -f segment -segment_time 10 -segment_list test.m3u8 -hls_key_info_file enc.keyinfo  test_%03d.ts


ffmpeg -y -i pikachu.mp4 -c:v libx264 -c:a libfdk_aac -bsf:v h264_mp4toannexb -hls_segment_filename "test_%03d.ts" -hls_playlist_type vod -hls_time 12   -hls_key_info_file enc.keyinfo test.m3u8
```