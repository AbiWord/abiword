Well, Jesper asked for this at GUADEC, so here it is.  The following
tarball contains the entire abiword tree, but none of the peer
directories, since they haven't been changed.  Here's how I would
reccomend playing with it:

  $ wget http://abisource.com/~sam/abi-auto.tar.gz
  $ mkdir abi-auto
  $ cd abi-auto
  $ tar -xzvf ../abi-auto.tar.gz
  $ ln -s ../wv wv
  $ ln -s ../psiconv psiconv
  $ ln -s ../expat expat
  $ pwd
  abi-auto
  $ ls
  abi expat psiconv wv
  $ cd abi
  $ ./autogen.sh [ this runs automake and autoconf ]  
  $ cd ..
  $ mkdir build
  $ cd build
  $ ../abi/configure
  $ make

When you're done, the binaries, called AbiWord_d and AbiWord_s, will
be located in abi/src/wp/main/unix/.  They should be directly runnable.  

Currently, no options to configure are supported.  I should have
--with-gnome and --with-debug done soon, though.

Tools required:
  autoconf (requires GNU m4)
  automake (requires perl)

Again, the tarball is at http://abisource.com/~sam/abi-auto.tar.gz

Have fun, and feel free to ask questions, or tell me how badly I
implemented this.  :-)
           
sam th --- sam@uchicago.edu --- http://www.abisource.com/~sam/
OpenPGP Key: CABD33FC --- http://samth.dyndns.org/key
DeCSS: http://samth.dyndns.org/decss

