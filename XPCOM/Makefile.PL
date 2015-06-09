# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is the Perl XPCOM language bindings.
#
# The Initial Developer of the Original Code is
# Jumpline.com, Inc. Portions created by Jumpline.com, Inc. are
# Copyright (C) 2001 Jumpline.com, Inc. All Rights Reserved.
#
# Contributor(s):
# Matt Kennedy <matt@jumpline.com> (original author)
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK *****

use ExtUtils::MakeMaker;

$gcc_cflags = '-fno-rtti -fno-exceptions -Wall -Wconversion -Wpointer-arith -Wbad-function-cast -Wcast-align -Woverloaded-virtual -Wsynth -pedantic -Wno-long-long -pipe -pthread -Wno-unused-function';
$gcc_libs = "-lplds4 -lplc4 -lnspr4 -lpthread -lxpcom -ldl -lm";
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'                  => 'XPCOM',
    'VERSION_FROM'          => 'XPCOM.pm', # finds $VERSION
    'DIR'                   => [ 'GenericModule', 'GenericFactory' ],
    'MAKEFILE'              => 'makefile.mk',
    'CC'                    => $ENV{CXX},
    'C'                     => ["perlXPUtil.cxx"],
    'OBJECT'                => 'Components.o perlXPUtil.o',
    'XS'                    => {'Components.xs'     => 'Components.c'},
    'CCFLAGS'               => "$gcc_cflags",
    'LIBS'                  => ["$gcc_libs"],

);
