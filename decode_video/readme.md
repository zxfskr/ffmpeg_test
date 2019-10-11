# 将h264文件解码为yuv文件

```
./test_g in.h264 out.yuv
ffplay -f rawvideo -video_size 320x568 out.yuv
```