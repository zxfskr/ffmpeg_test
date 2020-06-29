# 将音频包解码为pcm格式

```
./test_g in.mp2 out.pcm

ffplay -f s16le -ar 44100 -ac 2 out.pcm
```
# 音频的两个声道数据是堆在一起的，注意声道数
