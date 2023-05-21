Debug build path should look like:

```
/Users/kevin/Library/Developer/Xcode/DerivedData/sichuanhua-ayzdascehkrqtfbnbeenqfilvvpe/Build/Products/Debug
```

Use docker ffmpeg image to convert all m4a files to mp3

```
for file in *.m4a; do docker run --rm -it -v $(pwd):/config linuxserver/ffmpeg -i "config/$file" -codec:a libmp3lame -qscale:a 2 "config/${file%.m4a}.mp3"; done
```