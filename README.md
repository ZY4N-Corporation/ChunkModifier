# ChunkModifier

With this program you can render **3D objects** (Wavefront .OBJ) into **Minecraft Region files** (.mca).

## Features
1. textures with alpha channel
2. materials (blockIDs) can be specified
4. multi threaded
5. insert into existing regions
6. 3D-objects can be
    - ➡️ ⬆️ translated
    - ↔️ ↕️ scaled
    - 🔄 🔃 rotated
7. command line ready

## Usage

<details>
<summary><b>Command line arguments</b> </summary>
<p>

mandatory:&nbsp;🔴\
optional:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;🟢

> 🔴 input directory for region (.mca) files
> ```bash
> -inputDir "string"
> ```
> 🔴 output directory for region (.mca) files
> 
> ```bash
> -outputDir "string"
> ```
> 🔴 obj filename
> 
> ```bash
> -objDir "string"
> ```
> 🔴 number of workerthreads
>
> ```bash
> -numThreads "integer"
> ```
> 🟢 center objects around (0, 0, 0) 
> 
> ```bash
> -center "boolean"
> ```
> 🟢 translate object by x, y and z offset
> 
> ```bash
> -translate "float,float,float"
> ```
> 🟢 scale object in x, y and z dimension
> 
> ```bash
> -scale "float,float,float"
> ```
> 🟢 scale object to specific x, y and z dimension 
> 
> ```bash
> -scaleTo "float,float,float"
> ```
> 🟢 rotate around x, y and z axis
>
> ```bash
> -rotate "float,float,float"
> ```


</p>
</details>

### Example Command
> ```bash
> ./ChunkModifier -outputDir "C:/minecraft/region/" -inputDir "C:/minecraft/region/"  -objDir "E:\Library\3D objects\GTA\gta.obj" -numThreads "12" -center "true" -translate "128,50,128" -rotate "20,-30,60"
> ```
> This command inserts the **gta.obj** into the region files contained in **"C:/minecraft/region/"**.\
> Before rendering the objects center is moved to (128, 50, 128) and the object is rotated around the x, y and z axis.

### Materials
<details>
<summary><b>Specifiying materials</b> </summary>
<p>

To specify the blocks that the ChunkModifier can place in Minecraft
you may alter the **'blocks.txt'** file in **'ChunkModifier/data/assets/'**.\
The file is structured like this:
```c++
blockID, tex_filename, r g b a
...
```

| name          | type       | mandatory   | meaning                                                   |
| ------------- | ---------- | ----------- | --------------------------------------------------------- |
| blockID       | string     | ✓           | minecraft blockID starting with 'minecraft:'              |
| tex_filename  | string     | ✓           | texture filename used to calculate average color of block |
| r g b a       | 4x Integer | ✗           | RGBA 0-255 overrides the texture average color if needed  |

### Example
```c++
minecraft:bookshelf, bookshelf.png
minecraft:bricks, brick.png, 146 99 86 255
...
```

In this example the average color of the **'minecraft:bookshelf'** is not specified so the program will try to load\

**'ChunkModifier/data/assets/bookshelf.png'**
and add the average color of the image to the **'blocks.txt'** file.\
If you want to add more blocks to the **'blocks.txt'** file you should move your Minecraft textures into
the **'assets'** folder so that the ChunkModifier can load them.

To have more control over the materials and their corresponding blocks you can specify the blockID for a material in the .mtl file like so:
```mtl
newmtl pig
Kd 0.900000 0.000000 0.488000
d 1.000000
blockID minecraft:pink_concrete
```

</p>
</details>

## Dependencies
- [zlib](https://zlib.net/)  for (un)compressing region data
- [stb_image](https://github.com/nothings/stb) for loading textures

## Examples

### **[Utah Teapot](https://graphics.stanford.edu/courses/cs148-10-summer/as3/code/as3/teapot.obj)**

<img src="https://user-images.githubusercontent.com/63503707/119498295-c0dd3080-bd65-11eb-9b5c-22e3fb0775d2.png" alt="Teapot_Minecraft" width="400" height="400"/>


### **[GTA-V](https://sketchfab.com/3d-models/map-gta5-f622784b2fa9453fb20821afb74a9cb6)** by [Rockstar](https://www.rockstargames.com/games/V)

<img src="https://user-images.githubusercontent.com/63503707/119673736-e3da1400-be3b-11eb-9ef7-ad406e65b2cc.png" alt="GTA_5_Minecraft" width="400" height="400"/>

### **[MMX](https://wintergatan.net/collections/download/products/mmx-cad-wintergatan-original-files)** by [Wintergatan](https://www.youtube.com/channel/UCcXhhVwCT6_WqjkEniejRJQ)

<img src="https://user-images.githubusercontent.com/63503707/119498472-f124cf00-bd65-11eb-87e8-86a4b7c0ef6f.png" alt="MMX_Minecraft" width="400" height="400"/>

### **[a non existent cat](https://thiscatdoesnotexist.com/)**

<img src="https://user-images.githubusercontent.com/63503707/119557409-7200bc80-bda0-11eb-990a-7401cf6fb781.png" alt="Cat_Minecraft" width="400" height="400"/>

## Installation
1. download (and compile) zlib and stb_image
3. clone GitHub repositiory
4. open in Viusal Studio
5. link zlib and include library headers
6. build ChunkModifier with Visual Studio
7. finished

## Acknowledgements
- [TriangleBoxIntersection](https://gist.github.com/Philipp-M/e5747bd5a4e264ab143824059d21c120) by [Philip-M](https://github.com/Philipp-M) and improved by [Jeroen Flipts](https://github.com/jflipts)
    - checks collision between Minecraft block and 3D-object mesh 
 - [user94729](https://git.omegazero.org/user94729) for advice
