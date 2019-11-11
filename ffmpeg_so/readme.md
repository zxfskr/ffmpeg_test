# ffmpeg 4.2,centos 7.2

文件夹下的bin/目录cp到/usr/bin目录下，
```
cd bin/
chmod +x ffmpeg
chmod +x ffprobe
cd ..
cp bin/* /usr/bin
```
文件夹下的lib/目录cp到/usr/lib目录下，
```
cp lib/* /usr/lib
ldconfig
```
验证ffmpeg安装
```
ffmpeg -h
```

有可能权限不足，到时请加上sudo权限执行命令
