# MusicalShades
A program that uses song information to distort a mesh via vertex shaders and perlin noise. Final project for RIT's IGME 670 and IGME 740 courses, conducted as one project with permission from both professors.

# NOTES AND ATTRIBUTIONS:
This project is an extension of sample code provided by professor Chao Peng for an IGME 740 assignment, which set up the frameworks for passing data to and from shaders, initializing the camera and mesh, and running several of the OpenGL callbacks. Sample shaders were also provided by the professor, as was the perlin noise function used in this project's main shader; however, the lighting calculations of the relevant shader(s) (i.e. "per_Vertex_CUSTOM") is original, as is the song data processing.
Controls for the program are displayed in the tutorials in the top-right on launch.

For legal reasons, .wav files are not included. In order to test with custom files, simply place a .wav in the project directory (or a subdirectory), then copy the path ("*\[subdirectory\\\\\]*\*.wav") into the source code under **ah.create({...})**. Proper file importing is a planned feature once I have time to enjoy working on this project.

This code makes use of the following libraries: 
> <a href=http://www.mega-nerd.com/libsndfile/>libsndfile</a>, which is released under the <a href=https://www.gnu.org/licenses/lgpl-3.0.txt>GNU Lesser General Public License</a>.
> 
><a href=https://www.fftw.org>fftw</a>, which is released under the <a href=https://www.gnu.org/licenses/old-licenses/gpl-2.0.html>GNU General Public License ver. 2.0</a> <a href=https://www.fftw.org/doc/License-and-Copyright.html>and beyond</a>
> 
> <a href=https://github.com/freeglut/freeglut>freeglut</a> and <a href=https://glew.sourceforge.net>glew</a>

To run this program, download the relevant libraries/headers and place them in the designated universal_library directory.


Several StackOverflow posts were cited in the making of this program, all of which are linked in the relevant sections of the source code.

Due to limitations of the in-built C++ sound interface library, this code only works with .wav files. I recommend using <a href=https://ffmpeg.org>ffmpeg</a> in order to <a href=https://www.wikihow.com/Install-FFmpeg-on-Windows>convert other</a> <a href=https://stackoverflow.com/questions/5784661/how-do-you-convert-an-entire-directory-with-ffmpeg>filetypes</a> into wavs.

Part of the gitignore for this project was copied from <a href=https://github.com/github/gitignore/blob/main/VisualStudio.gitignore>the VS gitignore, as listed on github</a>.
