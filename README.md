# Minesweeperizer

Turn a video into a minesweeper.

## Prerequisite

You need these libraries installed on your machine

- SDL2
- SDL2_image
- OpenCV

## Instruction

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

Example run command

```
./minesweeperizer --file /home/andraantariksa/Projects/minesweeperizer/assets/itstimetostop720p.mp4 --greenscreen 44,255,16 --tolerance 50
```

The color are using RGB format `red,green,blue`, each field are ranging from 0 to 255.
Tolerance is the green screen color difference tolerance, the field are ranging from 0 to `sqrt(255^2+255^2+255^2)` which equal to  441.

For more information, run the program with `--help` flag.

## Preview

[![](http://img.youtube.com/vi/4TlQVZyvX4E/0.jpg)](http://www.youtube.com/watch?v=4TlQVZyvX4E "")

## Credit

- Assets: https://github.com/rkalis/minesweeper

## TODO

- Add audio support using FFmpeg

## License

The code is [MIT Licensed](LICENSE), not including the assets used.
