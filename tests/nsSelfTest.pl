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

# An example script demonstrating the inline implementation of XPCOM 
# interfaces (e.g. Making components in a script, instead of a perl module 
# placed in mozilla's components directory.
package nsSelfTest2::SampleImpl;
use strict;
use XPCOM;

sub new {
  my $self = { value  => PR_FALSE };
  bless $self, 'nsSelfTest2::SampleImpl';
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

package nsSelfTest2::Factory;
use strict;
use XPCOM;
#use nsSample::SampleImpl;

sub new {
  my $self = {};
  bless $self, 'nsSelfTest2::Factory';
  $Components::returnCode = Components::results::NS_OK;
  return $self;
}

sub createInstance {
  my ($pkg, $outer, $iid) = @_;
  if($outer) {
    $Components::returnCode = Components::results::NS_ERROR_NO_AGGREGATION;
    return undef;
  }
  my $sample = new nsSelfTest2::SampleImpl();
  $Components::returnCode = Components::results::NS_OK;
# Don't QI it here! That gets taken care of internally.
  return $sample;#->QueryInterface($iid);
}

package main;
use XPCOM;

my $factoryObject = new nsSelfTest2::Factory();
my $factory =  Components::WrapObject($factoryObject,
                                      $Components::interfaces{nsIFactory});
die "Unable to create factory." unless NS_SUCCEEDED;

my $compMgr = Components::Manager()
                -> QueryInterface(
                      $Components::interfaces{nsIComponentManagerObsolete});
die "Unable to obtain component manager." unless NS_SUCCEEDED;

my $cid = Components::ID('{00361b8b-6bfd-4f8c-a700-f14603655ccf}');
my $clsid = '@jumpline.com/perl-supports-bool;1';

$compMgr->registerFactory($cid, "Perl Bool", $clsid, $factory, PR_TRUE);
die "Unable to register factory." unless NS_SUCCEEDED;

my $bool = Components::Constructor($clsid,
                                  $Components::interfaces{nsISupportsPRBool});
die "Unable to create instance of component." unless NS_SUCCEEDED;
$bool->data(PR_TRUE);
print $bool->toString()."\n";

$compMgr->unregisterFactory($cid, $factory);

