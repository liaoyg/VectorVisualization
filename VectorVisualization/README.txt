
Example execution:
./volic ../data/helix_float.dat -g --transfer=../colortables/color001.png \
        --noise=../noise/noiseVol-256 --filter=../kernel/gaussKernel.png 

(For Visual Studio you have to set up the command arguments for debugging 
in the project properties)



Arguments of volic
==================

volic <volfilename.dat> [-h | --help] [-g | --gradient]
                        [-f <file> | --filter=<file>]
                        [-n <file> | --noise=<file>]
                        [-t <file> | --transfer=<file>]

<volfilename.dat>

The data set should be given in DAT/RAW format where the dat file
describes the dataset and the raw file contains the binary vector
data.

Sample dat file:

ObjectFileName: helix.raw
Resolution:     256 256 256
Format:         FLOAT3
SliceThickness: 1 1 1

Additionally UCHAR3 as data format is supported.


 -h | --help     Show usage

 -g | --gradient Use noise gradients

This parameter is necessary for gradient based illumination to ensure
the noise texture is created with 4 channels. Otherwise only a 3D noise
texture with one channel will be used.


 -f <png>        Filter kernel stored in PNG file
 --filter=<png>

The given png defines the filter kernel used in the LIC computation.
The file should be in grayscale otherwise only the red channel will 
be used. Only one line is mandatory for a valid filter, all other
lines will be ignored. If no filter is given, a box filter will be
used.


 -n <noisefile>  Use given noise for LIC
 --noise=<noisefile>

This is file contains the noise necessary for the LIC. The header of
the file contains three integer (32 bit each) describing the dimension 
of the noise data set. The data, one scalar per voxel, is subsequently 
stored in binary as unsigned character. 


 -t <png>        Transfer function stored in PNG file
 --transfer=<png>

The given filename is first expanded by volic and then two colortables
are loaded. Before the ".png" extension "_rgba" or "_alpha" is inserted.

Example:     given filename    colortable.png
             --> first table   colortable_rgba.png
                 second table  colortable_alpha.png

The first colortable contains rgb and alpha values (RGB or RGBA), the second
contains values for LIC opacity and optionally alpha (grayscale with
optional alpha).



Interaction
===========


Mouse
-----

The data set is rotated by a left mouse click according to a trackball
camera. With the middle mouse button the data set can be moved. The
distance to the camera is altered with the right mouse button.

The rotation can be snapped to 45 degree angles when SHIFT is pressed
additionally. 

If a clip plane or the light is active, the rotation and distance can
be changed by pressing CTRL and the left or right mouse button,
respectively. 


Keyboard
--------

Main Application:

q       quits the application

F1      activates vector data set shader (no LIC)
F2      activates LIC ray casting
F3      activates LIC slicing
F4		activates LIC Volume Precomputation ray casting
F5		switch animation mode

7       the streamlines are illuminated according to Zoeckler et al.
8       the streamlines are illuminated according to Mallo et al.
9       gradient-based illumination is used 
r       forces a reload of the GLSL shaders (no illumination applied)

t       toggles the visibility of the transfer function editor
H       toggles the HUD
w       shows a wireframe bounding box of the data set

1,2,3   toggle the state of three clip planes
4       deselects the current selected clip plane
5       toggles the visibility of the light position
0       stores a screenshot in "screenshot.png"

F       activates Framebuffer Objects with Float16 precision
space   switches to continously rendering the LIC instead of displaying
        an image after the last changes took place

LIC parameters
[       halves the step width for volume rendering
]       doubles the step width for volume rendering
a/z     increases/decreases the LIC step width
s/x     increases/decreases the number of LIC steps (forward)
S/X     increases/decreases the number of LIC steps (backward)
g/b     increases/decreases gradient scaling to improve contrast 
        between streamlines
h/n     increases/decreases frequency scaling
j/m     increases/decreases illumination scaling


Transfer Function Editor:

moue    draw color curves into the active channels
t       toggles the visibility of the editor
p       updates the transfer function according to the editor
s       shows the histogram of the data set magnitudes

r,g,b   activates the red/green/blue channel
a       activates the alpha channel (mapped to the magnitude of the
        data set)
o       activate the channel for LIC opacity

i       loads the identity into active channels
c       loads a constant value into active channels
m       mirrors the active channels
h       raises the values all active channels
n       lowers the values all active channels



example program arguement 
..\data\GeoDataAni\out.64.dat -g --transfer=..\colortables\color001.png --noise=..\noise\noise_256_80 --filter=..\kernel\gaussKernel.png 
..\data\HiroGeodata\out_64_0_magnetic_field.dat -g --transfer=..\colortables\color001.png --noise=..\noise\noise_256_80 --filter=..\kernel\gaussKernel.png 