A small program to apply various effects to .ppm images.

USAGE
1. The first argument is the path to the P6 .ppm image you want to load.
   
2. If you want to save the result use the "-s" flag followed by the path to the output file.
   The output format can be in .png, but it depends on imagemagick to do the conversion.
   If you dont have imagemagick installed, save the output as .ppm

FULL COMMAND EXAMPLE
./ppm-efx images/arch.ppm -s output.ppm/.png (conversion to .png depends on imagemagick)

CONTROLS
[Q] = toggle color quantization. While in this mode by pressing [Q] UP/DOWN changes the bit-depth.
[S] = toggle saturation. While in this mode by pressing [S] UP/DOWN changes the saturation value.
[I] = toggle color inversion.
[E] = toggle exposure. While in this mode by pressing [E] UP/DOWN changes the exposure value.
[C] = toggle contrast. While in this mode by pressing [C] UP/DOWN changes the contrast value.
[W] = toggle color warping. While in this mode by pressing [W],
      pressing [1], [2], [3] or [4] changes the warp mode to mirror vertical, mirror horizontal, tear horizontal or tear veritcal.
[D] = toggle dithering. While in this mode by pressing [D] UP/DOWN changes the brightness pre-dithering.
[M] = toggle monochrome. While in this mode by pressing [M], pressing [T] toggles thresholding and UP/DOWN changes the threshold.
[Z] = toggle color-shift. While in this mode by pressing [Z] UP/DOWN changes the color-shift.
[X] = toggle color-bias. While in this mode by pressing [R], [G] or [B] changes the bias to red, green or blue.
