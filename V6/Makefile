

all:


webinstall:
	rsync -avzhe ssh initramfs/www/* root@$(RPI_HOST):/www/

copyid: 
	  ([ ! -f ~/.ssh/id_rsa.pub ] && ssh-keygen); ssh-copy-id root@$(RPI_HOST)

terminal:
	ltsp-localapps xterm 
