This Docker image is based on dockerfile/ubuntu-desktop to provide a GUI desktop for running the BLLIP parser's NLTK + X11/TK parsing shell.  It starts up in CLI mode though for those who don't want the GUI.

The home for the BLLIP parser PyPi package bllipparser is:
	https://pypi.python.org/pypi/bllipparser/

Once you've started the Docker container with this image you should be able to  just type in those commands without any further installation steps.

The home for this Dockerfile is: https://github.com/BLLIP/bllip-parser/tree/master/dockerfiles/python
	
The source for the BLLIP parser (including Python wrapper) used is from here: https://github.com/BLLIP/bllip-parser
		
I add Firefox and Lynx to the minimal desktop for web access and NumPy since some 
NLTK features can use it.	
	
Once you've installed Docker, starting me is just:
```
docker run -it --rm -p 5901:5901 bllip/bllip-parser-python
```

Note that the Docker Hub Registry means you don't even need a manual download step.

Then at the prompt you can start VNC if you want graphics:
```
./runvnc.sh
```
That will prompt you for a password and start up the VNC server.

On a Mac an easy way to use the builtin VNC client is to open an OS X Terminal and type:
```
open vnc://192.168.59.103:5901
```
Or just enter the URL [[vnc://192.168.59.103:5901]] in the Safari location bar.
 
That (192.168.59.103) is the default VirtualBox IP address.  You can display it with this command:
```
boot2docker ip
```
When you're done using this application container, just exit from at the shell prompt:
```
[ root@4683cabf356e:~ ]$ exit
```

Docker greatly simplifies running Linux applications and works on most any platform including
Mac and Windows.  
	http://docs.docker.com/installation/

To run Docker on Mac or Windows, follow the instructions for installing boot2docker, which 
is mostly just a few clicks to install deal, here:
	http://docs.docker.com/installation/mac/
	http://docs.docker.com/installation/windows/
