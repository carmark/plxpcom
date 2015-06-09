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
# Damien Krotkine <dams@idm.fr> (some corrections)
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

package XPCOM;
use strict;
use vars qw($VERSION @ISA $AUTOLOAD @EXPORT_OK %EXPORT_TAGS);
use Exporter;
use DynaLoader;
use AutoLoader;

@ISA = qw(Exporter AutoLoader DynaLoader);
%EXPORT_TAGS = ( consts => [ qw(PR_TRUE PR_FALSE NS_SUCCEEDED) ], );
$EXPORT_TAGS{all} = [ map { @$_ } values %EXPORT_TAGS ];
@EXPORT_OK = map { @$_ } values %EXPORT_TAGS;
$VERSION = '0.81';
sub PR_TRUE { 1 }
sub PR_FALSE { 0 }
sub NS_SUCCEEDED { Components::isSuccessCode($Components::returnCode) }

#---------------
package XPCOM::nsISupports;
use strict;
use vars qw(@ISA @EXPORT $AUTOLOAD @EXPORT_OK);
use AutoLoader;

@ISA = qw(AutoLoader);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.
# Get the function name and insert it in between the object instance and its 
# args then invoke callXPCOMMethod

sub AUTOLOAD { XPCOM::nsISupports::callXPCOMMethod (shift, ($AUTOLOAD =~ /.*::(.*)$/), @_) }

#------------------
package nsID;
use strict;
use vars qw(@ISA $AUTOLOAD);
use AutoLoader;

@ISA = qw(AutoLoader);

sub AUTOLOAD { $_[0]->LookupIFaceConstant(($AUTOLOAD =~ /.*::(.*)$/)) }

#------------------
package Components;
use strict;
use vars qw($returnCode %interfaces %classes $self);
$self = undef;
$returnCode = 0;

tie %interfaces, 'XPCOM::ifaceHash', \&Components::Interfaces;
tie %classes, 'XPCOM::ifaceHash', \&Components::Classes;

#------------------
package XPCOM::ifaceHash;
use strict;

sub TIEHASH { bless $_[1], $_[0] }
sub FETCH { $_[0]->($_[1]) }

#------------------
package XPCOM;
bootstrap XPCOM $VERSION;

1;

__END__
# Below is the stub of documentation for your module. You better edit it!

=head1 NAME

XPCOM::Components - Perl interface to the XPCOM Component Manager.

=head1 SYNOPSIS

  use XPCOM qw(:all);
 
  $aClass = $Components::classes{'@mydomain.com/some-component;1'};
  $iFace = $aClass->QueryInterface($Components::interfaces{'someIFace'});
  $iFace->someFunction();

=head1 DESCRIPTION

This module provides access to the XPCOM Component Manager and Service 
Manager from Perl. From this module a perl script can create an instance of 
any scriptable XPCOM component. The typical method of obtaining a new 
component is to retrieve an instance of it from the hash 
C<%Components::classes>. For example:

  $aClass = $Components::classes{'@mydomain.com/some-component;1'};

You can also use the older method, which is to use the Components::Classes() 
function (Note the difference in case):

  $aClass = Components::Classes('@mydomain.com/some-component;1');
  
Once this call is made, you are still not ready to use the functions of the 
component. You must first call QueryInterface() on the new object to get an 
instance of the object implementing a particular interface. This is because 
a given object can support multiple interfaces (in fact, they all do, since 
every component supports nsISupports as well as at least one derived 
interface). Getting an interface is done thusly:

  $anInterface = 
      $aClass->QueryInterface($Components::interfaces{"anInterfaceName"});

Following this call, you can now use $anInterface to call methods of the 
requested interface. If QueryInterface() fails (e.g. the class doesn't support 
that interface), it will return undef. Note that you obtain an interface ID 
from the C<%Components::interfaces> hash. This returns the interface ID 
object for the interface. The QueryInterface() method still supports the 
older call method of simply providing an interface name as a string, but this 
method is deprecated and its use in new code should be discouraged.

The XPCOM::Components module has other functions as well, which we'll now 
discuss.

  $aClass = Components::ClassesByID("aClassID");

This is the same as Classes() with the exception being that it accepts the 
string for of the classes CID instead of its contract ID.

  $aService Components::GetService("contractID", 
                                   $Components::interface{"anInterfaceName"});

This function obtains a new instance of a service component from the 
Service Manager. It takes two string arguments. This first is the service's 
contract ID, the second is the interface name of the service. Because you 
are providing both the contract ID and interface, you do not need to call 
QueryInterface on the returned object, it is usable right away. For example, 
the following code shows how to obtain an instance of the Viper Socket 
Service:

  $sockService = Components::GetService(
                                  '@jumpline.com/network/socket-service;1',
                                  $Components::interfaces{"vpISocketService"});
  $aSocket = $sockService->getOpenSocket(1, "jumpline.com", 80);

The following method provides a means of obtaining the interface ID of an 
object:

  $iid = $Components::interfaces{'someIFaceName'};

This is the 'proper' means of refering to interfaces for methods such as 
QueryInterface().

This package also provides serveral methods for dealing with error checking 
and XPCOM method result codes. While typically invisible to scripting 
languages, B<every> XPCOM method returns a status code. Whenever you invoke 
a method of an XPCOM object, this result is stored in the variable 
$XPCOM::Components::returnCode. XPCOM has a very complex error report code 
organization (error codes are unsigned longs). To make things easy, the 
following function will return whether or not a return code is a success or 
error code:

  Components::isSuccessCode($retval)

If the return code is a success code this function returns true, otherwise it 
returns false. 

In addition, several common response codes are represented as constants under 
the XPCOM::Components::results namespace. For example, the following code 
demonstrates the proper means of creating an object and querying the 
interface with proper error checking:

  my $class = Components::Classes('@somedomain.com/some-class;1');
  if(!defined($class)) { print "Error\n"; exit; }
  my $obj = $class->QueryInterface($Components::interfaces{'someIFace'});
  if(!Components::isSuccessCode($Components::returnCode)) {
    # Checks if it's a no interface error
    if($Components::returnCode  == Components::results::NS_ERROR_NO_INTERFACE) {
      print "No such interface.\n";
    } else {
      print "An error occured in QueryInterface()\n";
    }
    exit;
  }
  ...
  
    
The module also implements a function called Constructor(), which is 
currently too annoying to recommend its use as you must provide it with the 
string form of the CID and IID of the component you wish to construct, which 
are typically not known to a script writer. This will be rectified at a later 
date.

=head1 AUTHOR

Matt Kennedy <matt@jumpline.com>

=head1 SEE ALSO

perl(1) XPCOM::nsISupports

=cut
