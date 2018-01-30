A variety of links to different application examples of `ADDA` and to related codes.



# Codes that use `ADDA` #
  * [HoloPy](http://launchpad.net/holopy) can be used as a python wrapper for `ADDA` ([tutorial](http://manoharan.seas.harvard.edu/holopy/users/dda_tutorial.html)).
  * [R wrapper for ADDA](https://github.com/baptiste/adda/wiki/wrapper_primer).
  * [Python wrapper for ADDA](https://github.com/aquilesC/plasmon_adda) to calculate spectra of nanoparticles ([tutorial](https://www.aquicarattino.com/science/plasmon-resonance/)).
  * [Python tool](https://github.com/yasokada/pySpherepts_171126) to generate spherical-quadrature points and wrappers/code snippets ([1](https://qiita.com/7of9/items/62151891b11b53857b46), [2](https://qiita.com/7of9/items/5c52189aba265d4c9d09)) to use them for orientation averaging with `ADDA`.

# Tools that operate with `ADDA` input/output #
Here we do not list tools/workflows that are supplied in [misc/](http://code.google.com/p/a-dda/source/browse/#svn%2Ftrunk%2Fmisc) folder ([short overview](PackageDescription.md)).

  * [LiteBil](http://users.abo.fi/jkniivil/litebil/) allows visualization of `ADDA` and `DDSCAT` shape files.

  * The following tools produce shape files for `DDSCAT` that can also be read by `ADDA`:
    * [DDSCAT File Conversion Tool](http://nanohub.org/resources/ddaconvert) converts `.obj` files to shape files. Similar to [misc/pip](http://a-dda.googlecode.com/svn/trunk/misc/pip/).
    * [DDA polyhedral targets](http://www.fisica.unam.mx/cecilia/codes.html) - a collection of codes to generate shape files of polyhedra.
    * [DDSCAT-tools](http://github.com/feranick/ddscat-tools) generate a sphere randomly covered by smaller spheres, and a slab with a sphere on top of it.
  * Python scripts to visualize particle shape, colored by x-position, [using Matplotlib](https://qiita.com/7of9/items/0ae5d739dd7276f3e1ca) or [using Povray](https://qiita.com/7of9/items/383dd7ffc630b31db082) (the latter is much faster).

# Other related (DDA) codes #
  * [DDSCAT](http://code.google.com/p/ddscat/) - popular general-purpose DDA code.
  * [OpenDDA](http://www.opendda.org/) - highly-optimized DDA code, uses both MPI and OpenMP parallelization.
  * [DDA-GPU](http://github.com/steffen-kiess/dda) - GPU-accelerated (using OpenCL) DDA code. Algorithms are partly based on `ADDA`.
  * [DDA-SI](http://code.google.com/p/dda-si/) - Matlab DDA code which includes surface interaction (by Sommerfeld integrals), but lacks FFT acceleration.

See also
  * [List of DDA codes at Wikipedia](http://en.wikipedia.org/wiki/Discrete_dipole_approximation_codes).
  * [Volume-integral section at Scattport](http://www.scattport.org/index.php/light-scattering-software/volume-integral).

# University courses #
  * [Electromagnetic Scattering II](http://wiki.helsinki.fi/display/53825/Electromagnetic+Scattering+II), University of Helsinki.
  * [Small bodies in the Solar system I, comets](http://wiki.helsinki.fi/display/PSR/Comets), University of Helsinki.
  * [Atmospheric Radiative Transfer](http://irina.eas.gatech.edu/EAS8803_SPRING2012/) ([lecture 18](http://irina.eas.gatech.edu/EAS8803_SPRING2012/Lec18.pdf)), Georgia Institute of Technology.

# Media #
  * [History of Russian market of open-source software](http://www.opennet.ru/docs/RUS/fss_history2/) (in Russian, May 2010)
