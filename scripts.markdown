About scripts
==========

Location and Naming Convention
----------

All Sanguosha scripts are written in QtScript, 
they should be suffixed with js, like init.js etc.
All built-in scripts are located in the scripts directory.

Built-in scripts
----------

* init.js, this scripts will be loaded first, initialize some kernel data.
* cards.js, which is used to describe the function of cards in Sanguosha's standard edition.
* generals.js, which is used to describe the skills and related data of generals in Sanguosha's standard edtion. 

Extension scripts
----------
For extension cards and extension generals, 
all resource and corresponding scripts should be located in extension directory, 
and each subdirectory of extension directory corresponds a extension package. 
The scripts named "init.js" under each subdirectory is called automatically when the package is loaded.

