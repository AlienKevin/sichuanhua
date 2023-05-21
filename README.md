Debug build path should look like:

```
/Users/kevin/Library/Developer/Xcode/DerivedData/sichuanhua-ayzdascehkrqtfbnbeenqfilvvpe/Build/Products/Debug
```

Use docker ffmpeg image in parallel to convert all m4a files to mp3

```
for file in M4A/*.m4a; do
    (result="MP3/${file#M4A/}"; result="${result%.m4a}.mp3"; docker run --rm -it -v $(pwd):/config linuxserver/ffmpeg -i "config/$file" -codec:a libmp3lame -qscale:a 2 "config/${result}";) & done
wait
```