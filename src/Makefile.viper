
############################################################################
# Makefile for vpIScriptEngine XPCOM component.
#
# This is a generic Makefile. You should be able to reuse it to build other 
# XPCOM components for viper. Simply adjust the next few lines for the 
# specifics of your component.
############################################################################

# Set the DEPTH, this is the relative position to the top of the viper 
# source tree.

USE_XPCOM=1
USE_PERL=1
EXTRA_CFLAGS= -DVIPER_MODULE_PATH=\"$(sandbox_modules)\"
# Set the component linker flags. As with the compiler flags, PERL_LDFLAGS,
# PERL_MODULE_LIBS, FLTK_LDFLAGS, and X_LDFLAGS are optional, but the others 
# should always be used.
#COMPONENT_LDFLAGS=$(LDFLAGS) $(XPCOM_LDFLAGS) $(XPCOM_LIBS) $(PERL_LDFLAGS) $(PERL_MODULE_LIBS) $(LIBS)

# Set this flag to 1 if you are embedding a perl interpreter. This will 
# generate an xsinit.cxx file. There should be no need for this in any 
# component but the script engine, so set it to 0.
EMBED_PERL=0

############## There should be no need to edit below this line #############
include $(DEPTH)/config/rules.mk

