# MusicalShades (*WIP as of 04/25/2024*)
A program that uses song information to distort a mesh via vertex shaders and perlin noise. Final project for RIT's IGME 670 and IGME 740 courses, conducted as one project with permission from both professors.

#NOTES:
This project is an extension of sample code provided by professor Chao Peng for an IGME 740 assignment, which set up the frameworks for passing data to and from shaders, initializing the camera and mesh, and running most of the OpenGL callbacks. Sample shaders were also provided by the professor, as was the perlin noise function used in the shader; however, the lighting calculations of the relevant shader(s) (i.e. "per_Vertex_CUSTOM") is original, as will be the song data processing.

This code makes use of libsndfile, which is released under the <a href=https://www.gnu.org/licenses/lgpl-3.0.txt>GNU Lesser General Public License</a>.

Several StackOverflow posts were cited in the making of this program, all of which are linked in the relevant sections of the source code.

Due to limitations of the in-built C++ sound interface library, this code only works with .wav files. I recommend using <a href=https://ffmpeg.org>ffmpeg</a> in order to <a href=https://www.wikihow.com/Install-FFmpeg-on-Windows>convert other</a> <a href=https://stackoverflow.com/questions/5784661/how-do-you-convert-an-entire-directory-with-ffmpeg>filetypes</a> into wavs.

Part of the gitignore for this project was copied from <a href=https://github.com/github/gitignore/blob/main/VisualStudio.gitignore>the VS gitignore, as listed on github</a>.
