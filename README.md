***** BEGIN LICENSE BLOCK *****
Version: MPL 1.1/GPL 2.0/LGPL 2.1

The contents of this file are subject to the Mozilla Public License Version
1.1 (the "License"); you may not use this file except in compliance with
the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the
License.

The Original Code is the Perl XPCOM language bindings.

The Initial Developer of the Original Code is
Jumpline.com, Inc. Portions created by Jumpline.com, Inc. are
Copyright (C) 2001 Jumpline.com, Inc. All Rights Reserved.

Contributor(s):
Matt Kennedy <matt@jumpline.com> (original author)

Alternatively, the contents of this file may be used under the terms of
either the GNU General Public License Version 2 or later (the "GPL"), or
the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
in which case the provisions of the GPL or the LGPL are applicable instead
of those above. If you wish to allow use of your version of this file only
under the terms of either the GPL or the LGPL, and not to allow others to
use your version of this file under the terms of the MPL, indicate your
decision by deleting the provisions above and replace them with the notice
and other provisions required by the GPL or the LGPL. If you do not delete
the provisions above, a recipient may use your version of this file under
the terms of any one of the MPL, the GPL or the LGPL.

***** END LICENSE BLOCK *****

Perl Interface to XPCOM
-----------------------

This package allows perl to access XPCOM objects as well as allowing XPCOM 
components to be implemented in Perl. This module is still young, and rough 
around the edges, but it is fairly functional in its current state.

Building the module
-------------------

Warning! These instructions are going to be Unix-centric until such time as 
I have time to build everything on Win32 and Mac.

These instructions cover building this module as a mozilla extension. The 
first step is to open the Perl XPCOM package into mozilla's extensions 
directory. Then you must edit the allmakefiles.sh file at the top of the 
mozilla source tree to include the extension into your build. The 
allmakefiles.sh file has a section for handling extensions, I recommend 
searching for the string 'p3p' in the file and it will take you where you 
need to go. You should find yourself at a switch statement block with the 
names of the directories in the extensions dir as the cases. Add the perl 
extension like this:

        # This section of code is already in allmakefiles.sh. I put it 
        # here to give you a reference for navigating this file.
        p3p ) MAKEFILES_extensions="$MAKEFILES_extensions
            extensions/p3p/Makefile
            extensions/p3p/public/Makefile
            extensions/p3p/resources/Makefile
            extensions/p3p/resources/content/Makefile
            extensions/p3p/resources/locale/Makefile
            extensions/p3p/resources/locale/en-US/Makefile
            extensions/p3p/resources/skin/Makefile
            extensions/p3p/src/Makefile
            " ;;
        # This is the part you need to add. Cut and paste this into 
        # allmakefiles.sh
        perl ) MAKEFILES_extensions="$MAKEFILES_extensions
            extensions/perl/Makefile
            extensions/perl/XPCOM/Makefile
            extensions/perl/src/Makefile
            extensions/perl/tests/Makefile
            extensions/perl/public/Makefile
            " ;;
        # Done

Once you've modified allmakefiles.sh, add the following line to the .mozconfig 
file in the top of the mozilla source tree:

  ac_add_options --enable-extensions=default,perl

Along with any other switches and extensions you'd like to add.

Once all that is done, you can build the lizard in the usual way (you have to use GNU make).

  make -f client.mk build

or

  ./configure && make

After Mozilla has finished building, the final step is to install the Perl 
module into Perl's include path. Do this by going into the 
extensions/perl/XPCOM directory and as root run the command:

  make module_install

Using the Extension
-------------------

I don't have a good, whiz-bang test yet. For now, the contents of the test 
directory build a component that replicates the nsSample implementation from 
the XPCOM tree. You can view the HTML page there, changing it to set the 
component contract ID to the perl component (look for '@mozilla.org/sample;1' 
and replace it with '@mozilla.org/perlsample;1'). Also included in the 
tests is the obligatory Hello World![tm]. Thrilling, eh...?

Bugs
----
You betcha.
What's implemented currently has worked without trouble. The two big gotchas 
to watch out for are:

Beware of invoking methods on Perl components from the destructor of a C++ 
object. I have found this to be the source of strange, random crashes which 
I haven't been able to track down yet.

Unicode is kludged, see this warning from the perlunicode(1) manpage:

  WARNING:  As of the 5.6.1 release, the implementation of Unicode
  support in Perl is incomplete, and continues to be highly experimental.

Because of this, I've chosen to force conversion of PRUnichar strings to 
C strings, which sucks.

Don't run a perl component outside of the main thread. Perl code can dispatch 
runnables to a thread, but the perl code itself cannot run in other threads. 
This is due to general thread unsafety in Perl and in not likely to change 
anytime soon.

Don't use external Perl modules that bind to C libraries because linking 
issues abound. By default, the perl component loader initializes its own
interpreter. Since this component is dynamically loaded by mozilla, symbols 
the perl modules need to bootstrap in the interpreter aren't exported. 
The workaround for this is to link libperl against the main application 
(only feasable if you're using XPCOM in an app other than mozilla) and 
clear the LOCAL_PERL Makefile variable in extensions/perl/src/Makefile. 
This will prevent the perl component loader from initializing its own 
interpreter.

TODO
----
Currently, I still haven't added Win32 makefiles.

Array types are partially implemented.

The XPCOM perl module package contains sub modules called XPCOM::GenericModule 
and XPCOM::GenericFactory which make it much less painful to create new 
components, unfortunately I haven't had time to write the POD docs for them 
yet!

Matt Kennedy <matt@jumpline.com>
Last Modified: Wed Oct 24 15:11:16 EDT 2001

