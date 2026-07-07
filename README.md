PPM EFX

A small C program to apply various effects to .ppm images. The result can be exported. Made while learning C. 


USAGE

You can either run the program with no arguments or with the file path to the P6 .ppm image you want to load. If you run the program with no arguments you will get prompted to enter the path to the image you want to load.

After closing the window, in the terminal you'll be asked if you want to save the result. If you answer yes, you'll get prompted to enter the file path to the output image, which can be a .ppm or a .png. Conversion to .png depends on imagemagick, so if you don't have imagemagick installed, simply save the output as a .ppm, or alternatively install it. 


FULL COMMAND EXAMPLE

"./ppm-efx images/arch.ppm -s output/cool_image.ppm"


GENERAL CONTROLS

[ESC] = reset position and scale.

[MIDDLE MOUSE CLICK & DRAG] = pan/move image.

[SCROLL WHEEL OR +/-] = scale image.

[H] = toggle the HUD visibility.


EFFECT CONTROLS

[Q] = toggle color quantization. While in this mode by pressing [Q] UP/DOWN changes the bit-depth.

[S] = toggle saturation. While in this mode by pressing [S] UP/DOWN changes the saturation value.

[I] = toggle color inversion.

[E] = toggle exposure. While in this mode by pressing [E] UP/DOWN changes the exposure value.

[C] = toggle contrast. While in this mode by pressing [C] UP/DOWN changes the contrast value.

[W] = toggle pixel warping. While in this mode by pressing [W],
      pressing [1], [2], [3] or [4] changes the warp mode to mirror vertical, mirror horizontal, upside down or sine-warp.
      Pressing UP/DOWN in this mode changes the sine wave's lenght, and LEFT/RIGHT changes the sine wave's ampiltude.
      
[D] = toggle dithering. While in this mode by pressing [D] UP/DOWN changes the brightness pre-dithering.

[M] = toggle monochrome. While in this mode by pressing [M], pressing [T] toggles thresholding and UP/DOWN changes the threshold.

[Z] = toggle color-shift. While in this mode by pressing [Z] UP/DOWN changes the color-shift.

[X] = toggle color-bias. While in this mode by pressing [X], [R], [G] and [B] changes the bias to red, green or blue.

[P] = toggle pixelation. While in this mode by pressing [P], UP/DOWN changes the pixel size.
