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


# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

######################### We start with some black magic to print on failure.

# Change 1..1 below to 1..last_test_to_print .
# (It may become useful if the test is moved to ./t subdirectory.)

BEGIN { $| = 1; print "1..1\n"; }
END {print "not ok 1\n" unless $loaded;}
use XPCOM;
$loaded = 0;
refcnt_test();
perl_component();
my $class = $Components::classes{'@mozilla.org/supports-string;1'};
if(defined($class)) {
  my $iface = $class->QueryInterface(
                                $Components::interfaces{'nsISupportsString'});
  if(defined($iface)) {
    $iface->data("Hello from XPCOM!\n");
    print $iface->toString();
    $loaded = 1;
  } else {
    $loaded = 0;
  }
} else {
  $loaded = 0;
}
print "ok 1\n";

sub refcnt_test {
  print "Scope test. If build with debug enabled, this test should be \n";
  print "followed by statements marking object destruction.\n";
  my $class = $Components::classes{'@mozilla.org/supports-wstring;1'};
  if(defined($class)) {
    my $iface = $class->QueryInterface(
                                  $Components::interfaces{'nsISupportsWString'});
    if(defined($iface)) {
      $iface->data("Hello from XPCOM in a subroutine!\n");
      print $iface->toString();
    }
  }
}

sub perl_component {
  print "Testing XPCOM perl component usage. If you have built support to\n";
  print "implement XPCOM components in perl and have nsSample.pm in your\n";
  print "components directory, then this test will try to create and use it.\n";
  
  my $class = $Components::classes{'@mozilla.org/perlsample;1'};
  if(defined($class)) {
    my $iface = $class->QueryInterface(
                                    $Components::interfaces{'nsISample'});
    if(defined($iface)) {
      $iface->writeValue("Hello XPCOM Perl Sample!\n");
    }
  }
}
######################### End of black magic.

# Insert your test code below (better if it prints "ok 13"
# (correspondingly "not ok 13") depending on the success of chunk 13
# of the test code):

