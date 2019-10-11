# 将音频包解码为pcm格式

```
./test_g in.mp2 out.pcm

ffplay -f s16le -ar 44100  out.pcm
```