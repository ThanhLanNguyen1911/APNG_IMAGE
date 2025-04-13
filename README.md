# APNG_IMAGE
apng importer C library
# Dependencies
<a href="https://github.com/nothings/stb/blob/master/stb_image.h">stbi_image.h</a>
# Description
APNG_IMAGE is an small header library that is written fully in C and it contains two functions for importing apng (animated png) files.The library will extract frames within the apng file and combine them into a single image of sequential frames <a href="https://youtu.be/B8hvxDKtILc">more info</a>.This is similar to how stb image does with gif files.The library depends on stb image functions for decoding the png and for vertical flip.The rest of the parser was written independently with extra features include gamma correction.
# Limitations
1.The width of the default image in pixel must be divisble for 4.<br/>
2.Frames must be identical in dimensions.<br/>
3.Support 8 bits color only.
# Manual
The library is straight forward,it contains two functions modularly:<br/>
<strong>apng_loaderM()</strong> is for loading from memory<br/>
<strong>apng_loaderF()</strong> is for loading from file name<br/>
both will return an unsigned char pointer to the final image and also write into parameters such as image width and height,bytes per pixel,number of frame,delay (fraction of 0.01 seconds unit).
There are three optional parameters such as:<br/>
1.flip 0 or 1<br/>
2.load_alpha 0 or 1<br/>
3.gamma 0 or 1<br/>
# global state
<strong>APNGI_LOAD_PNG</strong> 0 or 1<br/>
define 1 for loading regular png without returning (default is 0)
