#!/usr/bin/perl 
# Cheesy example script which prints all registered contract ID's. Written to 
# demonstrate getting a handle on the global component manager.
# 
# I omitted the license blurb cuz it'd be longer than the script itself. Do 
# what thou wilt.
use XPCOM;

my $compMgr = Components::Manager();
die "Unable to obtain component manager." unless NS_SUCCEEDED;

my $compRef = $compMgr->QueryInterface($Components::interfaces{nsIComponentManagerObsolete});
my $enum = $compRef->enumerateContractIDs();

my $item = $enum->currentItem();
my $ok = (NS_SUCCEEDED)?1:0;
my $clsid;
while($ok) {
  $clsid = $item->QueryInterface($Components::interfaces{nsISupportsString});
  if(NS_SUCCEEDED) {
    print $clsid->toString()."\n";
  }
  $enum->next();
  if(NS_SUCCEEDED) {
    $item = $enum->currentItem();
  } else {
    $ok = 0;
  }
}
