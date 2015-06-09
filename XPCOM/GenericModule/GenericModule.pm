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

package XPCOM::GenericModule;

#require 5.005_62;
use strict;
use warnings;
use XPCOM;

require Exporter;

use vars qw($VERSION);
$VERSION = '0.81';

sub new {
  my ($pkg) = @_;
  my $self = { myComponents => [] };
  bless $self, $pkg;
  return $self;
}

sub registerSelf {
  print "In GenericModule registerSelf.\n";
  my ($pkg, $compMgr, $fileSpec, $location, $type) = @_;
  $Components::returnCode = Components::results::NS_OK;
  foreach my $class ( @{$pkg->{myComponents}}) {
    my $compReg =
      $compMgr->QueryInterface($Components::interfaces{nsIComponentManagerObsolete});
    $compReg->registerComponentWithType($class->{myCID},
                                        $class->{myClassName},
                                        $class->{myContractID},
                                        $fileSpec,
                                        $location,
                                        PR_TRUE,
                                        PR_TRUE,
                                        $type);
    if(!Components::isSuccessCode($Components::returnCode)) {
      print "Error registering ".$class->{myClassName}."\n";
    }
  }
}

sub getClassObject {
  my ($pkg, $compMgr, $cid, $iid) = @_;
  my $theClass = undef;
  foreach my $class ( @{$pkg->{myComponents}}) {
    if($cid->equals($class->{myCID})) { 
      $theClass = $class;
      last; 
    }
  }
  if(defined($theClass)) {
    if(!$iid->equals($Components::interfaces{'nsIFactory'})) {
      $Components::returnCode = Components::results::NS_ERROR_NOT_IMPLEMENTED;
      return undef;
    }
    $Components::returnCode = Components::results::NS_OK;
    return $theClass->{myFactory};
  }
  $Components::returnCode = Components::results::NS_ERROR_NO_INTERFACE;
  return undef;
}
# Preloaded methods go here.

1;
__END__
# Below is stub documentation for your module. You better edit it!

=head1 NAME

XPCOM::GenericModule - Perl extension for blah blah blah

=head1 SYNOPSIS

  use XPCOM::GenericModule;
  blah blah blah

=head1 DESCRIPTION

Stub documentation for XPCOM::GenericModule, created by h2xs. It looks like the
author of the extension was negligent enough to leave the stub
unedited.

Blah blah blah.

=head2 EXPORT

None by default.


=head1 AUTHOR

A. U. Thor, a.u.thor@a.galaxy.far.far.away

=head1 SEE ALSO

perl(1).

=cut
