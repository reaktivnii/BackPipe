# BackPipe (Backend Pipeline)

This project exists as a personal tool for batch resizing of photos with a template.

It wants an input file or directory, an output path, path to a config and optionally a custom amount of threads (default is 6).

It creates a directory with a config's name at output path, renames every photo to add config's name at the end.

---

#### How to use it:

The binary file will be created in ./bin directory, use it like this:  
backpipe -c yourconfig.txt -i input -o output/  
optionally you can add -t for the amount of threads to use.  
  
It also can act as a service, use flag -s and give it an input ***directory*** (along with other main parameters) to launch it as one,
it will wait for inotify actions in the input dir (just copy or move photos to the dir) and automatically apply the template to new photos.
  
---

##### Full description of flags:

-c / --config - path to a config file (example how to make it below);  
-i / --input - input file or directory;  
-o / --output - output directory;  
-t / --threads - custom amount of threads to use (give your amount of physical threads, giving more will not make it go faster);  
-s / --service - launch BackPipe as a service.  
  
---
  
##### Config uses a simple logic:

[config_name]:  
[should aspect ratio be kept from the original]  
[result's width]  
[result's height, if aspect ratio is false]  
[what format the result should be, supports png and jpg]  
[if it's a jpg, quality from 0 to 100]  

example:  
save_aspect=false  
width=2000  
height=1335  
format=jpg  
quality=95  

The amount of these config bits in a single txt file isn't limited.  
It should be a plain text file.



Image manipulation done with nothings/stb libraries (image, resize2, write)
