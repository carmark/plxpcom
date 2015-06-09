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

###############################################################################
# File: plPerlBool.pm
# Created by: matt
# Created on: Thu 17 May 2001 09:16:30 PM EDT
# $Id: plPerlBool.pm,v 1.3 2002/02/19 19:44:35 matt Exp $
###############################################################################
# An example module that implements the nsISupportsPRBool interface.
# Simple, cheesy demonstration.
#
package plPerlBool;
use strict;
use XPCOM;

sub GetModule {
  my ($compMgr, $fileSpec) = @_;
  my $module = new plPerlBool::Module;
  $Components::returnCode = Components::results::NS_OK;
  return $module;
}

package plPerlBool::SampleImpl;
use strict;
use XPCOM;

sub new {
  my $self = { value  => PR_FALSE };
  bless $self, 'plPerlBool::SampleImpl';
  $Components::returnCode = Components::results::NS_OK;
  return $self;
}

sub data {
  my ($pkg, $aData) = @_;
  $Components::returnCode = Components::results::NS_OK;
  if(!$aData) {
    return $pkg->{value};
  }
  $pkg->{value} = $aData;
}

sub toString {
  my ($pkg) = @_;
  $Components::returnCode = Components::results::NS_OK;
  return (($pkg->{value})?"true":"false");
}

sub QueryInterface {
  my ($pkg, $iid) = @_;
  if(!$iid->equals($Components::interfaces{nsISupportsPRBool}) &&
     !$iid->equals($Components::interfaces{nsISupports})) {
    $Components::returnCode = Components::results::NS_ERROR_NO_INTERFACE;
  } else {
    $Components::returnCode = Components::results::NS_OK;
  }
  return $pkg;
}

package plPerlBool::Factory;
use strict;
use XPCOM;
#use plPerlBool::SampleImpl;

sub new {
  my $self = {};
  bless $self, 'plPerlBool::Factory';
  $Components::returnCode = Components::results::NS_OK;
  return $self;
}

sub createInstance {
  my ($pkg, $outer, $iid) = @_;
  if($outer) {
    $Components::returnCode = Components::results::NS_ERROR_NO_AGGREGATION;
    return undef;
  }
  my $sample = new plPerlBool::SampleImpl();
  $Components::returnCode = Components::results::NS_OK;
# Don't QI it here! That gets taken care of internally.
  return $sample;#->QueryInterface($iid);
}

package plPerlBool::Module;
use strict;
use XPCOM;
#use plPerlBool::Factory;

sub new {
  my $self = 
     { 
       firstTime  => 1,
       myCID      => Components::ID('{30da0283-a566-44a9-8d6c-09500b27a74e}'),
       myProgID   => '@mozilla.org/perl-supports-bool;1',
       myFactory  => new plPerlBool::Factory()
     };
  bless $self, 'plPerlBool::Module';
  $Components::returnCode = Components::results::NS_OK;
  return $self;
}

sub registerSelf {
  my ($pkg, $compMgr, $fileSpec, $location, $type) = @_;
  if($pkg->{firstTime} == 1) {
    $pkg->{firstTime} = 0;
    $Components::returnCode = 
      Components::results::NS_ERROR_FACTORY_REGISTER_AGAIN;
  }
  my $compReg = 
    $compMgr->QueryInterface($Components::interfaces{nsIComponentManagerObsolete});
  $compReg->registerComponentWithType($pkg->{myCID},
                                      "Sample Perl Component",
                                      $pkg->{myProgID},
                                      $fileSpec,
                                      $location,
                                      PR_TRUE,
                                      PR_TRUE,
                                      $type);
  $Components::returnCode = Components::results::NS_OK;
}

sub getClassObject {
  my ($pkg, $compMgr, $cid, $iid) = @_;
  if(!$cid->equals($pkg->{myCID})) {
    $Components::returnCode = Components::results::NS_ERROR_NO_INTERFACE;
    return undef;
  }
  if(!$iid->equals($Components::interfaces{'nsIFactory'})) {
    $Components::returnCode = Components::results::NS_ERROR_NOT_IMPLEMENTED;
    return undef;
  }
  $Components::returnCode = Components::results::NS_OK;
  return $pkg->{myFactory};
}

1;
__END__
