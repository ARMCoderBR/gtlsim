////////////////////////////////////////////////////////////////////////////////
// GTLSIM
// A TINY LOGIC CIRCUIT SIMULATOR
// (C) 2020 BY ARMCODER - milton@armcoder.com.br
//
// https://github.com/ARMCoderBR/gtlsim
//
// THIS PROGRAM IS FREE SOFTWARE
// SEE LICENSE AT https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
////////////////////////////////////////////////////////////////////////////////

    Version 1.1.0
    July 01, 2020

    INTRO
    ~~~~~
    
    Hi folks!

    GTLSim is a follow-up to TLSIM, a pet project that I decided to build after
discovering Ben Eater's YouTube channel. At his channel, Ben holds a series of
videos where he teaches how to build a very simple computer over some using just
TTL ICs, switches, LEDs and other discrete parts.

    For reference, the playlist with the complete tutorial is here:
   
    https://www.youtube.com/playlist?list=PLowKtXNTBypGqImE405J2565dvjafglHU
   
    Just for the record, I'm not affiliated with Ben Eater in any way, this is
simply a software inspired by his work, although I definitely recommend to check
his YouTube channel. He's a great teacher in the field of electronics, IMO.

    Like TLSIM, this project aims to accurately simulate Ben's implementation 
through software. The difference here is that GTLSIM is built over GTK, whilst
TLSIM was built on the old text-based NCurses library.

    The old TLSIM project is no longer supported, except for bugfixes that may
be needed, and can be found at:

    https://github.com/ARMCoderBR/tlsim     

         
    ENVIRONMENT
    ~~~~~~~~~~~    
    
    This is (for now) a Linux-only project, since it was only tested in Ubuntu
20.04. I used Eclipse 2019-09 over OpenJDK 11, and the 'gtk-3' and 'pthread' 
additional libraries are required to build.

    However, in order to Eclipse build the project properly, I needed to install
the 'pkg-config' plugin and then check the 'gtk+-3.0' box in the page made
available by the plugin. Without the plugin, the build configuration can become
very difficult.

    
    BEN'S COMPUTER VARIANTS
    ~~~~~~~~~~~~~~~~~~~~~~~
    
    Some fans created hardware projects that extended the functionality beyond 
the original design (more instructions, more memory, etc). They are easy to find
on Youtube. I have no plans, for now, to cater to these variants.
    
    
    QUICKSTART
    ~~~~~~~~~~

    The EEPROM's microcode is pre-initialized, and the RAM contents is 
automatically initialized by the 'exmachina.c' module that fires a thread that
simulates the needed user inputs to fill the RAM with a sample program. 

    After that the computer is started, but can be stopped and the RAM program
can be edited by the user through the UI using the mouse to flip the 'dip
switches' on the board.
    
    How each module works is explained in detail in the Ben's videos, I have
replicated them quite faithfully in the UI.

    The bottom row in the UI interface has 4 buttons, they are:
    
    * Pause/Step Clock
    
    * Unpause/Slower Clock
    
    * Unpause/Faster Clock
    
    * Exit
    
    That's for now.

    Stay tuned and have fun!
    
    
    CHANGELOG
    ~~~~~~~~~
    
    V1.1.0 - July 01, 2020:
    
    - Gtlsim 1.1.0!
