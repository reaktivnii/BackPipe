# BackPipe (Backend Pipeline)

This project exists as a personal tool for batch resizing of photos with a template.

It wants an input file or directory, an output path, path to a config and optionally a custom amount of threads (default is 6).

It creates a directory with a config's name at output path, renames every photo to add config's name at the end.

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
